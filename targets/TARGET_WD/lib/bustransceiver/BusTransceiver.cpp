/* 
* BusTransceiver.cpp
*
* Created: 28.07.2017 09:59:39
* Author: Patrick.Frischmann
*/

#include "BusTransceiver.h"
#include "wd_logging.h"

#define BT_EOF_CHAR_MATCH			(126)
#define BT_RX_CHECK_INTERVAL_SEC	0.1

extern "C" {
	// there is no extern "C" in lib_crc.h, so we have to put it here
	// because I don't want to modify the .h file
	#include "lib_crc.h"
}

rtos::Mutex BusTransceiver::_mutex;

BusTransceiver::BusTransceiver(PinName Tx, PinName Rx, int baud /*= MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE*/)
	: _bt_rx_consumer(0), _bt_rx_producer(0), _queue() {

	this->_dmaSerial = new DMASerial(Tx, Rx, baud);
	
	// start event-thread
	this->_eventThread.start(callback(&_queue, &EventQueue::dispatch_forever));
		
	// init
	this->_bt_rx_buffer = new char[BT_RX_BUFFER_SIZE]();
	this->_bt_tx_buffer = new char[BT_TX_BUFFER_SIZE]();
	
	// configure DMA usage
	this->_dmaSerial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	this->_dmaSerial->set_dma_usage_tx(DMA_USAGE_ALWAYS);
	
	this->_bt_timeout = new Ticker();
    this->_bt_timeout->attach(
	    _queue.event(callback(this, &BusTransceiver::_bt_rx_frame_received)),
	    BT_RX_CHECK_INTERVAL_SEC
    );
	
}

BusTransceiver::~BusTransceiver() {
	
	delete this->_bt_rx_buffer;
	delete this->_bt_tx_buffer;
	delete this->_bt_timeout;
	delete this->_dmaSerial;
	
} //~BusTransceiver


void BusTransceiver::_bt_tx_complete(int evt) {

	wd_log_debug("_bt_tx_complete");

}

void BusTransceiver::_bt_rx_complete(int evt) {
	
	wd_log_debug("_bt_rx_complete");
	
}

void BusTransceiver::_bt_rx_frame_received(void) {
	
	if (!_mutex.trylock()) { 
		return; // method is called periodically anyway
	}
	
	char buf[BT_RX_BUFFER_SIZE] = { };
	
	int remainingBytes = this->_dmaSerial->GetLength();
	this->_bt_rx_producer = BT_RX_BUFFER_SIZE - remainingBytes;
	
	int length = 0;

	if (this->_bt_rx_consumer < this->_bt_rx_producer) { // normal
		length = this->_bt_rx_producer - this->_bt_rx_consumer;
		memcpy(buf, this->_bt_rx_buffer + this->_bt_rx_consumer, length);
	} 
	else if (this->_bt_rx_consumer > this->_bt_rx_producer) { // overrun
		length = BT_RX_BUFFER_SIZE - this->_bt_rx_consumer;
		memcpy(buf, this->_bt_rx_buffer + this->_bt_rx_consumer, length);
		
		memcpy(buf + length, this->_bt_rx_buffer, this->_bt_rx_producer);
		length += this->_bt_rx_producer;
	}
	else {
		_mutex.unlock();
		return;
	}
	
	if (this->_bt_rx_buffer[((this->_bt_rx_producer + BT_RX_BUFFER_SIZE) - 1) % BT_RX_BUFFER_SIZE] != BT_EOF_CHAR_MATCH) {
		wd_log_debug("Incomplete frame, continue!");
		_mutex.unlock();
		return;
	}
	
	this->_bt_rx_consumer = this->_bt_rx_producer;
	
	if (length < 5) {// crc + delimiter
		wd_log_warn("Frame is too short to be valid! (length: %d)", length);
		_mutex.unlock();
		return;
	}
	
	// Maybe more frames?
	
	int buffer_start = 0;
	int buffer_length = 0;
	
	for (int i = 0; i < length; i++) {
		
		if (buf[i] == BT_EOF_CHAR_MATCH){
			
			// crc check
			uint32_t crc = UINT32_MAX;
			for (int j = buffer_start; j < i; j++) {
				crc = update_crc_32(crc, buf[j]);
			}
			wd_log_debug("BusTranceiver: Start-index: %d, CRC length: %d, first-char: %c, last-char %c", buffer_start, i - buffer_start, buf[buffer_start], buf[i - buffer_start - 1]);
			
			if (crc != 0 && (i + 1) < length) {
				// continue probably '~' in payload
				wd_log_debug("BusTranceiver: CRC error, probably '~' in payload");
				continue;
			}
			
			buffer_length = i - buffer_start;
	
			bool isEcho = memcmp(this->_bt_tx_buffer, buf + buffer_start, buffer_length) == 0;
		
			if (crc != 0) { // CRC error
				wd_log_error("BusTranceiver: CRC error, discarding frame! (length: %d, crc: %x, first byte: %x)", buffer_length, crc, buf[buffer_start]);	
			}
			else if (isEcho) { // Tx echo
				wd_log_debug("BusTranceiver: Received echo, discarding frame! (length: %d, crc: %x, first byte: %x)", buffer_length, crc, buf[buffer_start]);
			}
			else { // valid frame
				wd_log_info("BusTranceiver: Received valid frame (length: %d, first byte: %x).", buffer_length, buf[buffer_start]);
				this->bt_handle_frame(buf + buffer_start, buffer_length - 4); // exclude crc in upper layer
			}
			
			buffer_start = i+1;
			
		}
		
	}
	
	_mutex.unlock();
		
}

void BusTransceiver::bt_start(void) {

	// start async read
	this->_dmaSerial->read(
		this->_bt_rx_buffer, 
		BT_RX_BUFFER_SIZE, 
		callback(this, &BusTransceiver::_bt_rx_complete)
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
	
	this->_dmaSerial->write(this->_bt_tx_buffer, size, callback(this, &BusTransceiver::_bt_tx_complete));
	
}

// implement this in the upper layer to handle valid frames
__weak void BusTransceiver::bt_handle_frame(const void * data, size_t size) {}
