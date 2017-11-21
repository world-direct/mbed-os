/* 
* BusTransceiver.cpp
*
* Created: 28.07.2017 09:59:39
* Author: Patrick.Frischmann
*/

#include "BusTransceiver.h"
#include "wd_logging.h"

extern "C" {
	// there is no extern "C" in lib_crc.h, so we have to put it here
	// because I don't want to modify the .h file
	#include "lib_crc.h"
}

BusTransceiver::BusTransceiver(PinName Tx, PinName Rx, PinName ActivityLed, int baud /*= MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE*/)
<<<<<<< HEAD
	: _tx_complete_sem(0),
	  _tx_echo_received_sem(0),
=======
	: _producer_pointer(0),
	  _tx_complete_sem(1),
	  _tx_echo_received_sem(1),
>>>>>>> Modified transmit routine for bus transceiver, should unblock bt_transmit_frame earlier but need an additional buffer
	  _bt_activity_led(ActivityLed, 1) {

	this->_dmaSerial = new DMASerial(Tx, Rx, baud);
	this->_bt_activity_led_timeout = new ResettableTimeout(callback(this, &BusTransceiver::_on_bt_activity_led_timeout), 500000);
		
	// init
	this->_bt_rx_buffer = new char[BT_BUFFER_SIZE]();
	this->_bt_tx_buffer = new char[BT_BUFFER_SIZE]();
	this->_bt_tx_frame_buffer = new char[BT_BUFFER_SIZE/2]();
	
	// configure DMA usage
	this->_dmaSerial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	this->_dmaSerial->set_dma_usage_tx(DMA_USAGE_ALWAYS);
	
	this->_dmaSerial->attachRxCallback(Callback<void(dma_frame_meta_t *)>(this, &BusTransceiver::_bt_rx_process_frame));
	
}

BusTransceiver::~BusTransceiver() {
	
	delete this->_bt_rx_buffer;
	delete this->_bt_tx_frame_buffer;
	delete this->_bt_tx_buffer;
	delete this->_dmaSerial;
	this->_txQueueProcessingThread.terminate();
	
} //~BusTransceiver


void BusTransceiver::_bt_tx_complete(int evt) {

	wd_log_debug("_bt_tx_complete");
	this->_tx_complete_sem.release();

}

void BusTransceiver::_bt_indicate_activity(void) {
	this->_bt_activity_led.write(0);
	this->_bt_activity_led_timeout->reset();
}

void BusTransceiver::_on_bt_activity_led_timeout(void) {
	this->_bt_activity_led.write(1);
}

void BusTransceiver::_tx_queue_process_loop(void) {
	
	while(true) {
		
		// we will wait here for frame reception
		osEvent evt = _dma_tx_frame_queue.get(osWaitForever);
		
		if (evt.status == osEventMail) {
			
			dma_frame_meta_t * frame_meta = (dma_frame_meta_t *) evt.value.p;
			
			size_t size;
			this->_dmaSerial->getFrame(frame_meta, this->_bt_tx_frame_buffer, &size);
			
			_dma_tx_frame_queue.free(frame_meta);
			
			// crc computation
			uint32_t crc = UINT32_MAX;
			
			for (size_t i = 0; i < size; i++) {
				char c = ((char*)this->_bt_tx_frame_buffer)[i];
				crc = update_crc_32(crc, c);
			}
			
			// write the crc
			union {
				uint32_t crc;
				char data[4];
			} crcdata = { crc };
			
			memcpy(this->_bt_tx_frame_buffer + size, crcdata.data, 4);
			size += 4;
			
			this->_tx_frame_size = size;
		
			// send frame
			this->_dmaSerial->write(this->_bt_tx_frame_buffer, size, callback(this, &BusTransceiver::_bt_tx_complete));
			
			// wait for transmit completion
			this->_tx_complete_sem.wait(BT_TX_WRITE_TIMEOUT);
			this->_tx_complete_sem.release();
			
			// wait for echo reception
			this->_tx_echo_received_sem.wait(BT_TX_ECHO_TIMEOUT);
			this->_tx_echo_received_sem.release();
			
		}
		
	}

}

void BusTransceiver::_bt_rx_process_frame(dma_frame_meta_t * frame_meta) {
	
	this->_bt_indicate_activity();
	
	if (frame_meta->frame_size < 5) {// crc already 4 bytes
		wd_log_warn("Frame is too short to be valid! (length: %d)", frame_meta->frame_size);
		return;
	}
	
	char rxBuffer[frame_meta->frame_size] = {};
	size_t rxSize;
	
	this->_dmaSerial->getFrame(frame_meta, rxBuffer, &rxSize);
	
	// crc check
	uint32_t crc = UINT32_MAX;
	for (int i = 0; i < rxSize; i++) {
		crc = update_crc_32(crc, rxBuffer[i]);
	}
	
	if (crc != 0) { 
		wd_log_error("BusTranceiver: CRC error, discarding frame! (length: %d, crc: %x, first byte: %x)", rxSize, crc, rxBuffer[0]);
		return;
	}
	
	// echo check
	if (rxSize == this->_tx_frame_size) { // only perform echo check if frame sizes match
		
		if (memcmp(this->_bt_tx_frame_buffer, rxBuffer, rxSize) == 0) { // Tx echo
			wd_log_debug("BusTranceiver: Received echo, discarding frame! (length: %d, crc: %x, first byte: %x)", rxSize, crc, rxBuffer[0]);
			this->_tx_echo_received_sem.release();
			return;
		}
		
	}
	
	// valid frame
	wd_log_info("BusTranceiver: Received valid frame (length: %d, first byte: %x).", rxSize, rxBuffer[0]);
	this->bt_handle_frame(rxBuffer, rxSize-4); // exclude crc in upper layer

}

void BusTransceiver::bt_start(void) {

	// start async read
	this->_dmaSerial->startRead(
		this->_bt_rx_buffer, 
		BT_BUFFER_SIZE
	);

	this->_txQueueProcessingThread.start(mbed::Callback<void()>(this, &BusTransceiver::_tx_queue_process_loop));

}

void BusTransceiver::bt_transmit_frame(const void * data, size_t size) {
	
	this->_bt_indicate_activity();
	
	unsigned int frame_start = this->_producer_pointer;
	if (frame_start + size < BT_BUFFER_SIZE) {
		memcpy(this->_bt_tx_buffer + frame_start, data, size);
		this->_producer_pointer += size;
	} else {
		int len = BT_BUFFER_SIZE - this->_producer_pointer;
		memcpy(this->_bt_tx_buffer + this->_producer_pointer, data, len);
		memcpy(this->_bt_tx_buffer, (uint8_t *)data + len, size - len);
		this->_producer_pointer = size - len;
	}
	
<<<<<<< HEAD
	// write the crc
	union {
		uint32_t crc;
		char data[4];
	} crcdata = { crc };
	memcpy(this->_bt_tx_buffer + size, crcdata.data, 4);
	size += 4;
	
	// send frame
	this->_dmaSerial->write(this->_bt_tx_buffer, size, callback(this, &BusTransceiver::_bt_tx_complete));
	
	// wait for transmit completion
	this->_tx_complete_sem.wait(BT_TX_WRITE_TIMEOUT);
	
	// wait for echo reception
	this->_tx_echo_received_sem.wait(BT_TX_ECHO_TIMEOUT);
=======
	dma_frame_meta_t * frame_meta = _dma_tx_frame_queue.alloc();
	if (frame_meta != NULL) {
		frame_meta->buffer = this->_bt_tx_buffer;
		frame_meta->buffer_size = BT_BUFFER_SIZE;
		frame_meta->frame_start_pos = frame_start;
		frame_meta->frame_size = size;
		if (_dma_tx_frame_queue.put(frame_meta) != osOK) {
			wd_log_error("BusTransceiver: Unable to enqueue frame!");
		}
	} else {
		wd_log_error("BusTransceiver: Error allocating memory for frame queue!");
	}
>>>>>>> Modified transmit routine for bus transceiver, should unblock bt_transmit_frame earlier but need an additional buffer
	
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
