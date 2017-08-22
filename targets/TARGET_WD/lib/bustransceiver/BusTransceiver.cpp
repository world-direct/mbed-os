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
	: _bt_rx_consumer(0), 
	  _bt_rx_producer(0),
	  _tx_semaphore(1) {

	this->_dmaSerial = new DMASerial(Tx, Rx, baud);
		
	// init
	this->_bt_rx_buffer = new char[BT_BUFFER_SIZE]();
	this->_bt_tx_buffer = new char[BT_BUFFER_SIZE]();
	this->_bt_rx_buffer_step = new char[BT_BUFFER_SIZE]();
	
	// configure DMA usage
	this->_dmaSerial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	this->_dmaSerial->set_dma_usage_tx(DMA_USAGE_ALWAYS);
	
	// start rx-thread
	this->_readProcessingThread.start(mbed::Callback<void()>(this, &BusTransceiver::_bt_rx_entry));
	
}

BusTransceiver::~BusTransceiver() {
	
	delete this->_bt_rx_buffer;
	delete this->_bt_tx_buffer;
	delete this->_bt_rx_buffer_step;
	delete this->_dmaSerial;
	
} //~BusTransceiver


void BusTransceiver::_bt_tx_complete(int evt) {

	wd_log_debug("_bt_tx_complete");
	this->_tx_semaphore.release();

}

void BusTransceiver::_bt_rx_complete(int evt) {
	
	wd_log_debug("_bt_rx_complete");
	
}

void BusTransceiver::_bt_rx_entry(void){

	while(true){
		
		this->_bt_rx_locked_step();
		Thread::wait(BT_RX_CHECK_INTERVAL_MSEC);
		
	}
	
}

void BusTransceiver::_bt_rx_locked_step(void) {
	
	if (!_mutex.trylock()) {
		return;
	}
	_bt_rx_step();
	_mutex.unlock();
	
}

void BusTransceiver::_bt_rx_step(void) {
	
	_stepTimer.reset();
	_stepTimer.start();
	
	do {
		this->_bt_rx_producer = BT_BUFFER_SIZE - this->_dmaSerial->GetLength();
		if (this->_bt_rx_producer == this->_bt_rx_consumer) {
			Thread::wait(BT_RX_READ_TIMEOUT1);
		}
	} while (
		this->_bt_rx_producer == this->_bt_rx_consumer && 
		_stepTimer.read_ms() < BT_RX_READ_TIMEOUT2);
	
	_stepTimer.reset();
	do {
		this->_bt_rx_producer = BT_BUFFER_SIZE - this->_dmaSerial->GetLength();
		Thread::wait(BT_RX_READ_TIMEOUT1);
	} while (
		(
			this->_bt_rx_producer != (BT_BUFFER_SIZE - this->_dmaSerial->GetLength()) ||
			this->_bt_rx_buffer[(BT_BUFFER_SIZE + (this->_bt_rx_producer - 1)) % BT_BUFFER_SIZE] != BT_EOF_CHAR_MATCH
		) &&
		_stepTimer.read_ms() < BT_RX_READ_TIMEOUT2);
	
	_stepTimer.stop();
	
	int length = 0;

	if (this->_bt_rx_consumer < this->_bt_rx_producer) { // normal
		length = this->_bt_rx_producer - this->_bt_rx_consumer;
		memcpy(_bt_rx_buffer_step, this->_bt_rx_buffer + this->_bt_rx_consumer, length);
	} 
	else if (this->_bt_rx_consumer > this->_bt_rx_producer) { // overrun
		length = BT_BUFFER_SIZE - this->_bt_rx_consumer;
		memcpy(_bt_rx_buffer_step, this->_bt_rx_buffer + this->_bt_rx_consumer, length);
		
		memcpy(_bt_rx_buffer_step + length, this->_bt_rx_buffer, this->_bt_rx_producer);
		length += this->_bt_rx_producer;
	}
	else {
		return;
	}
	
	if (this->_bt_rx_buffer[((this->_bt_rx_producer + BT_BUFFER_SIZE) - 1) % BT_BUFFER_SIZE] != BT_EOF_CHAR_MATCH) {
		wd_log_debug("Incomplete frame, continue!");
		return;
	}
	
	this->_bt_rx_consumer = this->_bt_rx_producer;
	
	if (length < 5) {// crc + delimiter
		wd_log_warn("Frame is too short to be valid! (length: %d)", length);
		return;
	}
	
	// Maybe more frames?
	
	int buffer_start = 0;
	int buffer_length = 0;
	
	for (int i = 0; i < length; i++) {
		
		if (_bt_rx_buffer_step[i] == BT_EOF_CHAR_MATCH) {
			
			// crc check
			uint32_t crc = UINT32_MAX;
			for (int j = buffer_start; j < i; j++) {
				crc = update_crc_32(crc, _bt_rx_buffer_step[j]);
			}
			wd_log_debug("BusTranceiver: Start-index: %d, CRC length: %d, first-char: %c, last-char %c", buffer_start, i - buffer_start, _bt_rx_buffer_step[buffer_start], _bt_rx_buffer_step[i - buffer_start - 1]);
			
			if (crc != 0 && (i + 1) < length) {
				// continue probably '~' in payload
				wd_log_debug("BusTranceiver: CRC error, probably '~' in payload");
				continue;
			}
			
			buffer_length = i - buffer_start;
	
			bool isEcho = memcmp(this->_bt_tx_buffer, _bt_rx_buffer_step + buffer_start, buffer_length) == 0;
		
			if (crc != 0) { // CRC error
				wd_log_error("BusTranceiver: CRC error, discarding frame! (length: %d, crc: %x, first byte: %x)", buffer_length, crc, _bt_rx_buffer_step[buffer_start]);	
			}
			else if (isEcho) { // Tx echo
				wd_log_debug("BusTranceiver: Received echo, discarding frame! (length: %d, crc: %x, first byte: %x)", buffer_length, crc, _bt_rx_buffer_step[buffer_start]);
			}
			else { // valid frame
				wd_log_info("BusTranceiver: Received valid frame (length: %d, first byte: %x).", buffer_length, _bt_rx_buffer_step[buffer_start]);
				this->bt_handle_frame(_bt_rx_buffer_step + buffer_start, buffer_length - 4); // exclude crc in upper layer
			}
			
			buffer_start = i + 1;
			
		}
		
	}
	
}

void BusTransceiver::bt_start(void) {

	// start async read
	this->_dmaSerial->read(
		this->_bt_rx_buffer, 
		BT_BUFFER_SIZE, 
		callback(this, &BusTransceiver::_bt_rx_complete),
		SERIAL_EVENT_RX_CHARACTER_MATCH,
		BT_EOF_CHAR_MATCH
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
	
	// append delimiter
	char delimiter = BT_EOF_CHAR_MATCH;
	memcpy(this->_bt_tx_buffer + size, &delimiter, 1);
	size++; 
	
	// send frame
	this->_dmaSerial->write(this->_bt_tx_buffer, size, callback(this, &BusTransceiver::_bt_tx_complete));
	this->_tx_semaphore.wait(BT_TX_WRITE_TIMEOUT);
	this->_bt_rx_locked_step();
	this->_tx_semaphore.release();
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
