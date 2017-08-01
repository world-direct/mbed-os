/* 
* BusTransceiver.cpp
*
* Created: 28.07.2017 09:59:39
* Author: Patrick.Frischmann
*/


#include "BusTransceiver.h"
#include "Mutex.h"
#include "wd_logging.h"

extern "C" {
	// there is no extern "C" in lib_crc.h, so we have to put it here
	// because I don't want to modify the .h file
	#include "lib_crc.h"
}

BusTransceiver::BusTransceiver(PinName Tx, PinName Rx, int baud /*= MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE*/)
	: RawSerial(Tx, Rx, baud), _bt_rx_consumer(0), _bt_rx_producer(0) {
	
	// init
	this->_bt_rx_pin  = new InterruptIn(Rx);
	this->_bt_rx_buffer = new char[BT_RX_BUFFER_SIZE]();
	this->_bt_tx_buffer = new char[BT_TX_BUFFER_SIZE]();
	
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

static Mutex _bt_rx_mutex;
static Mutex _bt_tx_mutex;

void BusTransceiver::_bt_rx_active_interrupt(void) {

	this->_bt_timeout->reset();

}

void BusTransceiver::_bt_tx_complete(int evt) {
	
	this->send_break();

}

void BusTransceiver::_bt_rx_complete(int evt) {} 

void BusTransceiver::_bt_rx_frame_received(void) {
	
	_bt_rx_mutex.lock();
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
	
	// crc check
	uint32_t crc = UINT32_MAX;
	for(int i = 0; i < length; i++) {
		crc = update_crc_32(crc, buf[i]);
	}
	
	bool isEcho = memcmp(this->_bt_tx_buffer, buf, length) == 0;
		
	_bt_rx_mutex.unlock();
	
	if (crc != 0) { // CRC error
		wd_log_error("CRC error, discarding frame! (length: %d, crc: %x, first byte: %x)", length, crc, buf[0]);
	} else if (isEcho) { // Tx echo
		wd_log_info("Received echo, discarding frame! (length: %d, crc: %x, first byte: %x)", length, crc, buf[0]);
	} else { // valid frame
		wd_log_info("Received valid frame (length: %d, first byte: %x).", length, buf[0]);
		this->bt_handle_frame(buf, length - 4); // exclude crc in upper layer
	}
	
}

void BusTransceiver::bt_start(void) {

	// start async read
	this->read(_bt_rx_buffer, BT_RX_BUFFER_SIZE, NULL); // callback(this, &BusTransceiver::_bt_rx_complete), SERIAL_EVENT_RX_COMPLETE);

}

void BusTransceiver::bt_transmit_frame(const void * data, size_t size) {
	
	_bt_tx_mutex.lock();
	
	uint32_t crc = UINT32_MAX;
	
	memcpy(this->_bt_tx_buffer, data, size);
	
	// crc computation
	for(size_t i = 0; i < size; i++) {
		char c = ((char*)data)[i];
		crc = update_crc_32(crc, c);
	}
	
	// write the crc
	union {
		uint32_t crc;
		char data[4];
	} crcdata = {crc};
	memcpy(this->_bt_tx_buffer + size, crcdata.data, 4);
	size += 4;
	
	_bt_tx_mutex.unlock();
	
	this->write(this->_bt_tx_buffer, size, callback(this, &BusTransceiver::_bt_tx_complete), SERIAL_EVENT_TX_COMPLETE);
	
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
