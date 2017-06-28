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

#ifndef SERIALSTREAMADAPTER_H_
#define SERIALSTREAMADAPTER_H_

#include <stdint.h>
#include "mbed.h"
#include "Cellular/core/IOStream.h"
#include "BufferedSerial.h"

#define READTIMEOUT 5

class SerialStreamAdapter : public IOStream
{
	
	private:
		BufferedSerial* _bufferedSerial;
	
	public: 
		
		SerialStreamAdapter(BufferedSerial* bufferedSerial);
		virtual int abortRead();
		virtual int read(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout /* = osWaitForever */);
		virtual int waitAvailable(uint32_t timeout /* = osWaitForever */);
		virtual int abortWrite();
		virtual int waitSpace(uint32_t timeout /* = osWaitForever */);
		virtual int write(uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */);
		virtual size_t space();
		virtual size_t available();
		virtual void attach(Callback<void()> func, RawSerial::IrqType type);
	
};

#endif // SERIALSTREAMADAPTER_H_