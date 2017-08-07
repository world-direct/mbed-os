/*
 * SerialStreamAdapter.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created: 21.04.2017
 * Author:	Simon Pfeifhofer
 * EMail:	simon.pfeifhofer@world-direct.at
 *
 * Description:
 *		Creates an IOStream-adapter for a BufferedSerial-instance.
 *
 */ 

#include "SerialStreamAdapter.h"
#include "wd_logging.h"
#include "Cellular/core/errors.h"

DMASerial* SerialStreamAdapter::_serial;
Mutex SerialStreamAdapter::_mutex;
Semaphore SerialStreamAdapter::complete_sem;

SerialStreamAdapter::SerialStreamAdapter(BufferedSerial* bufferedSerial) {
	wd_log_debug("SerialStreamAdapter --> ctor");
	this->_bufferedSerial = bufferedSerial;
	
	SerialStreamAdapter::_serial = new DMASerial(GSM_TXD, GSM_RXD, 115200);
	SerialStreamAdapter::_serial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	SerialStreamAdapter::_serial->set_dma_usage_tx(DMA_USAGE_NEVER);
	
}

int SerialStreamAdapter::abortRead() {
	wd_log_debug("SerialStreamAdapter --> SerialStreamAdapter read aborted");
	return 0;
}

int SerialStreamAdapter::abortWrite() {
	wd_log_debug("SerialStreamAdapter --> abortWrite");
	return 0;
}

int SerialStreamAdapter::waitAvailable(uint32_t timeout /* = osWaitForever */) {
	//TODO: severity solution, too many log-entries:	wd_log_debug("SerialStreamAdapter --> abortWrite");
	return this->_bufferedSerial->readable() > 0 ? 0 : 1;
}

size_t SerialStreamAdapter::available() {
	wd_log_debug("SerialStreamAdapter --> available");
	return 0;
}

int SerialStreamAdapter::waitSpace(uint32_t timeout /* = osWaitForever */) {
	wd_log_debug("SerialStreamAdapter --> waitSpace");
}

size_t SerialStreamAdapter::space() {
	wd_log_debug("SerialStreamAdapter --> space");
	return 0;
}

void SerialStreamAdapter::read_callback(int a){
	SerialStreamAdapter::complete_sem.release();
}

void SerialStreamAdapter::write_callback(int a) {
}

static int buffersize = 512;
static char dma_buffer[512];
static int dma_consumer_pointer = 0;
static bool reading_started = false;


void SerialStreamAdapter::start_reading() {
	SerialStreamAdapter::_serial->read(dma_buffer, buffersize, callback(SerialStreamAdapter::read_callback));
	reading_started = true;
}

int SerialStreamAdapter::read(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout /* = osWaitForever */) {
	
	if(!SerialStreamAdapter::_mutex.trylock()){
		Thread::wait(timeout);
		return NET_TIMEOUT;
	}
	
	if(!reading_started){
		start_reading();
	}
	
	Timer timer;
	timer.start();
	
	while (timer.read_ms() < timeout && !SerialStreamAdapter::complete_sem.wait(100)) {}
	
	// timeout or callback occurred!!
	int remainingBytes = SerialStreamAdapter::_serial->GetLength();
	int dma_producer_pointer = buffersize - remainingBytes;
	
	int length = 0;
	if (dma_consumer_pointer < dma_producer_pointer) {
		// normal
		memcpy(buf, dma_buffer + dma_consumer_pointer, dma_producer_pointer - dma_consumer_pointer);
		length = dma_producer_pointer - dma_consumer_pointer;
		wd_log_error("normal");
	}
	
	if (dma_consumer_pointer > dma_producer_pointer) {
		// overrun
		memcpy(buf, dma_buffer + dma_consumer_pointer, buffersize - dma_consumer_pointer);
		length = buffersize - dma_consumer_pointer;
		memcpy(buf + (buffersize - dma_consumer_pointer), dma_buffer, dma_producer_pointer);
		length += dma_producer_pointer;
		wd_log_error("overrun");
	}
	
	// overflow currently not handled!
	dma_consumer_pointer = dma_producer_pointer;
	*pLength = length;
	
	SerialStreamAdapter::_mutex.unlock();
	
	if (length == 0) {
		return NET_TIMEOUT;
	}
	
	return OK;
}

int SerialStreamAdapter::write(uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */) {
	wd_log_debug("SerialStreamAdapter --> write");
	
//  SerialStreamAdapter::_serial->write(buf, length, callback(SerialStreamAdapter::write_callback));
	for(int i = 0; i < length; i++){
		SerialStreamAdapter::_serial->putc(buf[i]);
	}
	
	//ssize_t effectiveLength = this->_bufferedSerial->write(buf, length);
	//if (effectiveLength < length) {
		//return NET_FULL;
	//}
	
	return OK;
}

void SerialStreamAdapter::attach(Callback<void()> func, RawSerial::IrqType type) {
	this->_bufferedSerial->attach(func, type);
}



