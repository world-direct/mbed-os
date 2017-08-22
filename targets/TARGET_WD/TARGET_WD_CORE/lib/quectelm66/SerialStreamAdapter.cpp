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

SerialStreamAdapter::SerialStreamAdapter() {
	wd_log_debug("SerialStreamAdapter --> ctor");
	_serial = new DMASerial(GSM_TXD, GSM_RXD, 115200);
	_serial->set_dma_usage_rx(DMA_USAGE_ALWAYS);
	_serial->set_dma_usage_tx(DMA_USAGE_NEVER);
}

int SerialStreamAdapter::abortRead() {
	wd_log_debug("SerialStreamAdapter --> SerialStreamAdapter read aborted");
	_serial->abort_read();
	return 0;
}

int SerialStreamAdapter::abortWrite() {
	wd_log_debug("SerialStreamAdapter --> abortWrite");
	_serial->abort_write();
	return 0;
}

int SerialStreamAdapter::waitAvailable(uint32_t timeout /* = osWaitForever */) {
	return 0;
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
	//SerialStreamAdapter::_complete_sem.release();
}

void SerialStreamAdapter::write_callback(int a) {
}

void SerialStreamAdapter::start_reading() {
	_serial->read(_dma_buffer, DMA_BUFFER_SIZE, callback(this, &SerialStreamAdapter::read_callback));
	_reading_started = true;
}

int SerialStreamAdapter::read(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout /* = osWaitForever */) {
	
	if(!SerialStreamAdapter::_mutex.trylock()){
		Thread::wait(timeout);
		return NET_TIMEOUT;
	}
	
	if(!_reading_started){
		start_reading();
	}
	
	Timer timer;
	timer.start();
	
	int dma_producer_pointer = 0;
	
	do{
		dma_producer_pointer = DMA_BUFFER_SIZE - SerialStreamAdapter::_serial->GetLength();
		if (dma_producer_pointer == _dma_consumer_pointer) {
			Thread::wait(DMA_READ_SEM_COMPLETE_TIMEOUT_MS < timeout ? DMA_READ_SEM_COMPLETE_TIMEOUT_MS : timeout);
		}
	} while (dma_producer_pointer == _dma_consumer_pointer && timer.read_ms() < timeout);
	
	// Handle ongoing DMA-transfer
	do{
		dma_producer_pointer = DMA_BUFFER_SIZE - SerialStreamAdapter::_serial->GetLength();
		Thread::wait(10);
	} while(dma_producer_pointer != (DMA_BUFFER_SIZE - SerialStreamAdapter::_serial->GetLength()));
	
	dma_producer_pointer = DMA_BUFFER_SIZE - SerialStreamAdapter::_serial->GetLength();
	int length = 0;
	if (_dma_consumer_pointer < dma_producer_pointer) {
		// normal
		memcpy(buf, _dma_buffer + _dma_consumer_pointer, dma_producer_pointer - _dma_consumer_pointer);
		length = dma_producer_pointer - _dma_consumer_pointer;
		wd_log_debug("SerialStreamAdapter --> normal dma read");
	}
	
	if (_dma_consumer_pointer > dma_producer_pointer) {
		// overrun
		memcpy(buf, _dma_buffer + _dma_consumer_pointer, DMA_BUFFER_SIZE - _dma_consumer_pointer);
		length = DMA_BUFFER_SIZE - _dma_consumer_pointer;
		memcpy(buf + (DMA_BUFFER_SIZE - _dma_consumer_pointer), _dma_buffer, dma_producer_pointer);
		length += dma_producer_pointer;
		wd_log_debug("SerialStreamAdapter --> overrun dma read");
	}
	
	// overflow currently not handled!
	_dma_consumer_pointer = dma_producer_pointer;
	*pLength = length;
	
	_mutex.unlock();
	
	if (length == 0) {
		return NET_TIMEOUT;
	}
	
	return OK;
}

int SerialStreamAdapter::write(uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */) {
	wd_log_debug("SerialStreamAdapter --> write");
	
	for(int i = 0; i < length; i++){
		_serial->putc(buf[i]);
	}
	
	// TODO: Replace with TX-DMA
	// _serial->write(buf, length, callback(this, &SerialStreamAdapter::write_callback));
	
	return OK;
}



