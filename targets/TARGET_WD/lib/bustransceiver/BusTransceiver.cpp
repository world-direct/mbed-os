/* 
* BusTransceiver.cpp
*
* Created: 28.07.2017 09:59:39
* Author: Patrick.Frischmann
*/


#include "BusTransceiver.h"
#include "Mutex.h"

BusTransceiver::BusTransceiver(PinName Tx, PinName Rx, int baud /*= MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE*/)
	: RawSerial(Tx, Rx, baud), _bt_rx_consumer(0), _bt_rx_producer(0) {
	
	// init
	this->_bt_rx_pin  = new InterruptIn(Rx);
	this->_bt_rx_buffer = new uint8_t[BT_RX_BUFFER_SIZE]();
	
	// configure DMA usage
	this->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	this->set_dma_usage_tx(DMA_USAGE_ALWAYS);
	
	/* Configure timeout for frame detection
	 * 
	 * The idea is to use a timeout, whose counter is reset in response to rising edges on an input capture
	 * connected to the USART receive pin.
	 * 
	 * When no data are received (i.e. no rising edge of USART Rx), the timer/counter continues its operation 
	 * until the defined timeout value is reached. The timeout value is configured according to the length of 
	 * the break signal. Consequently, an interrupt is generated once a frame ends.
	 * 
	 * */
	this->_bt_rx_pin->fall(callback(this, &BusTransceiver::_bt_rx_active_interrupt));	// fall because otherwise we get the interrupt twice
	this->_bt_rx_pin->enable_irq();
	timestamp_t timout_us = this->_bt_break_time_us()*0.8f;
	this->_bt_timeout = new ResettableTimeout(callback(this, &BusTransceiver::_bt_rx_frame_received), timout_us);
	
}

// default destructor
BusTransceiver::~BusTransceiver() {
	
	delete this->_bt_rx_buffer;
	delete this->_bt_timeout;
	delete this->_bt_rx_pin;
	
} //~BusTransceiver

static Mutex _mutex;

void BusTransceiver::_bt_rx_active_interrupt(void) {

	this->_bt_timeout->reset();

}

void BusTransceiver::_bt_tx_complete(int evt) {
	
	this->send_break();

}

void BusTransceiver::_bt_rx_complete(int evt) {} 

void BusTransceiver::_bt_rx_frame_received(void) {
	
	_mutex.lock();
	char buf[BT_RX_BUFFER_SIZE] = {};
	
	struct serial_s *obj_s = &((this->_serial).serial);
	DMA_HandleTypeDef *hdma = &DmaRxHandle[obj_s->index];
	
	int remainingBytes = __HAL_DMA_GET_COUNTER(hdma);
	this->_bt_rx_producer = BT_RX_BUFFER_SIZE - remainingBytes;
	
	int length = 0;
	if (this->_bt_rx_consumer < this->_bt_rx_producer) { // normal
		length = this->_bt_rx_producer - this->_bt_rx_consumer;
		memcpy(buf, this->_bt_rx_buffer + this->_bt_rx_consumer, length);
	} 
	else if (this->_bt_rx_consumer > this->_bt_rx_producer) { // overrun
		
		memcpy(buf, this->_bt_rx_buffer + this->_bt_rx_consumer, BT_RX_BUFFER_SIZE - this->_bt_rx_consumer);
		length = BT_RX_BUFFER_SIZE - this->_bt_rx_consumer;
		memcpy(buf + length, this->_bt_rx_buffer, this->_bt_rx_producer);
		length += this->_bt_rx_producer;
	}
	else {
		// overflow currently not handled!	
	}
	
	this->_bt_rx_consumer = this->_bt_rx_producer;
	
	_mutex.unlock();
	
	this->bt_handle_frame(buf, length);
	
}

void BusTransceiver::bt_start(void) {

	// start async read
	this->read(_bt_rx_buffer, BT_RX_BUFFER_SIZE, callback(this, &BusTransceiver::_bt_rx_complete), SERIAL_EVENT_RX_COMPLETE);

}

void BusTransceiver::bt_transmit_frame(const void * data, size_t size) {
	
	this->write((uint8_t *)data, size, callback(this, &BusTransceiver::_bt_tx_complete), SERIAL_EVENT_TX_COMPLETE);
	
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
