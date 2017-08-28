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

BusTransceiver::BusTransceiver(PinName Tx, PinName Rx, int baud /*= MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE*/)
	: _tx_complete_sem(1),
	  _tx_echo_received_sem(1) {

	this->_dmaSerial = new DMASerial(Tx, Rx, baud);
		
	// init
	this->_bt_rx_buffer = new char[BT_BUFFER_SIZE]();
	this->_bt_tx_buffer = new char[BT_BUFFER_SIZE]();
	
	// configure DMA usage
	this->_dmaSerial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	this->_dmaSerial->set_dma_usage_tx(DMA_USAGE_ALWAYS);
	
	this->_dmaSerial->attachRxCallback(Callback<void(dma_frame_meta_t *)>(this, &BusTransceiver::_bt_rx_process_frame));
	
}

BusTransceiver::~BusTransceiver() {
	
	delete this->_bt_rx_buffer;
	delete this->_bt_tx_buffer;
	delete this->_dmaSerial;
	
} //~BusTransceiver


void BusTransceiver::_bt_tx_complete(int evt) {

	wd_log_debug("_bt_tx_complete");
	this->_tx_complete_sem.release();

}

void BusTransceiver::_bt_rx_process_frame(dma_frame_meta_t * frame_meta) {
	
	if (frame_meta->frame_size < 5) {// crc already 4 bytes
		wd_log_warn("Frame is too short to be valid! (length: %d)", frame_meta->frame_size);
		return;
	}
	
	char buffer[frame_meta->frame_size] = {};
	size_t length;
	
	this->_dmaSerial->getFrame(frame_meta, buffer, &length);
	
	// crc check
	uint32_t crc = UINT32_MAX;
	for (int i = 0; i < length; i++) {
		crc = update_crc_32(crc, buffer[i]);
	}
	
	if (crc != 0) { // CRC error
		wd_log_error("BusTranceiver: CRC error, discarding frame! (length: %d, crc: %x, first byte: %x)", length, crc, buffer[0]);
	}
	else if (memcmp(this->_bt_tx_buffer, buffer, length) == 0) { // Tx echo
		wd_log_debug("BusTranceiver: Received echo, discarding frame! (length: %d, crc: %x, first byte: %x)", length, crc, buffer[0]);
		this->_tx_echo_received_sem.release();
	}
	else { // valid frame
		wd_log_info("BusTranceiver: Received valid frame (length: %d, first byte: %x).", length, buffer[0]);
		this->bt_handle_frame(buffer, length-4); // exclude crc in upper layer
	}

}

void BusTransceiver::bt_start(void) {

	// start async read
	this->_dmaSerial->startRead(
		this->_bt_rx_buffer, 
		BT_BUFFER_SIZE
	);

}

void BusTransceiver::bt_transmit_frame(const void * data, size_t size) {
	
	uint32_t crc = UINT32_MAX;
	
	memcpy(this->_bt_tx_buffer, data, size);
	
	// crc computation
	for (size_t i = 0; i < size; i++) {
		char c = ((char*)data)[i];
		crc = update_crc_32(crc, c);
	}
	
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
	this->_tx_complete_sem.release();
	
	// wait for echo reception
	this->_tx_echo_received_sem.wait(BT_TX_ECHO_TIMEOUT);
	this->_tx_echo_received_sem.release();
	
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
