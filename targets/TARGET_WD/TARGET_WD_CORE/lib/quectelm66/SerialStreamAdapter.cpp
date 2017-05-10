#include "SerialStreamAdapter.h"
#include "wd_logging.h"
#include "Cellular/core/errors.h"

SerialStreamAdapter::SerialStreamAdapter(BufferedSerial* bufferedSerial) {
	wd_log_debug("SerialStreamAdapter --> ctor");
	this->_bufferedSerial = bufferedSerial;
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

int SerialStreamAdapter::read(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout /* = osWaitForever */) {
	
	Timer timer;
	timer.start();
	
	size_t idx = 0;
	while (idx < maxLength && (timeout == 0 || timer.read_ms() < timeout)) {
		
		if (!this->_bufferedSerial->readable()) {
			if (timeout == 0) {
				*pLength = idx;
				return idx > 0 ? OK : NET_EMPTY;
			}
			__WFI();
			continue;
		}
		
		if (maxLength - idx) {
			buf[idx++] = this->_bufferedSerial->getc();
		}
		
	}
	
	*pLength = idx;
	
	if (idx >= maxLength) {
		return NET_FULL;
	}
	
	if (idx == 0) {
		return NET_TIMEOUT;
	}
	
	return OK;
}

int SerialStreamAdapter::write(uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */) {
	wd_log_debug("SerialStreamAdapter --> write");
	
	ssize_t effectiveLength = this->_bufferedSerial->write(buf, length);
	if (effectiveLength < length) {
		return NET_FULL;
	}
	
	return OK;
}

void SerialStreamAdapter::attach(Callback<void()> func, RawSerial::IrqType type) {
	this->_bufferedSerial->attach(func, type);
}



