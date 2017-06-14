#include "DS18B20.h"
#include <utility>

#define DS18B20_POWER_PARASITE	0x00
#define DS18B20_POWER_EXTERN	0x01

#define DS18B20_COMMAND_CONVERT_T				0x44
#define DS18B20_COMMAND_WRITE_SCRATCHPAD		0x4E
#define DS18B20_COMMAND_READ_SCRATCHPAD			0xBE
#define DS18B20_COMMAND_COPY_SCRATCHPAD			0x48
#define DS18B20_COMMAND_RECALL_E2				0xB8

#define DS18B20_COPYSP_DELAY      10 /* ms */
#define DS18B20_SP_SIZE		8	// size without crc
#define DS18B20_TH_REG      2
#define DS18B20_TL_REG      3

#define DS18B20_9_BIT		0
#define DS18B20_10_BIT		(1<<5)
#define DS18B20_11_BIT		(1<<6)
#define DS18B20_12_BIT		((1<<6)|(1<<5))

// conversion times in ms
#define DS18B20_TCONV_12BIT      750
#define DS18B20_TCONV_11BIT      DS18B20_TCONV_12_BIT/2
#define DS18B20_TCONV_10BIT      DS18B20_TCONV_12_BIT/4
#define DS18B20_TCONV_9BIT       DS18B20_TCONV_12_BIT/8

#define NUM_RETRIES		3


DS18B20::DS18B20(OneWire * oneWire, const Callback<void(uint64_t)> & sensorAddedCallback, const Callback<void(uint64_t)> & sensorRemovedCallback)
	: _sensorAddedCallback(sensorAddedCallback), _sensorRemovedCallback(sensorRemovedCallback), _ticker() {
	
	this->_oneWire = oneWire;

	// start event queue dispatch thread
	this->_eventThread.start(callback(&_queue, &EventQueue::dispatch_forever));
		
	this->enumerateSensors();
		
	this->_ticker.attach(callback(this, &DS18B20::collectMeasurement), (float)(DS18B20_MEASUREMENT_INTERVAL_S));
}


DS18B20::~DS18B20() {}

OW_STATUS_CODE DS18B20::enumerateSensors(void) {
	
	char sensorIds[OW_MAXSENSORS][OW_ROMCODE_SIZE];
	int retry_count = NUM_RETRIES;
	
	// start by searching for sensors
	OW_STATUS_CODE res;
	do {
		res = this->_oneWire->ow_search_sensors(&this->_sensorCount, &sensorIds[0][0], DS18B20_FAMILY_CODE);
		if (res == OW_OK && this->_sensorCount > 0) break;
	} while (--retry_count > 0);
	
	// return if search was unsuccessful
	if (res != OW_OK) return res;
	
	// clear our sensor map and queue notification(s) if no sensors were found
	if (this->_sensorCount == 0) {
		
		for(map<uint64_t, DS18B20MeasurementBuffer>::const_iterator it = this->_mSensors.begin(); it != this->_mSensors.end(); it++) {
			_queue.call(this->_sensorRemovedCallback, it->first);
		}
		
		this->_mSensors.clear();
		return OW_OK;
	}
	
	/* sync detected sensors with map */
	
	// remove sensors no longer available on OneWire bus
	for(map<uint64_t, DS18B20MeasurementBuffer>::const_iterator it = this->_mSensors.begin(); it != this->_mSensors.end(); it++) {
		
		bool stillPresent = false;
		for (int i = 0; i<this->_sensorCount; i++){
			if (it->first == this->transformId(sensorIds[i])) {
				stillPresent = true;
				break;
			}
		}
		
		if (!stillPresent) {
			_queue.call(this->_sensorRemovedCallback, it->first);
			this->_mSensors.erase(it->first);
		}
	}
	
	// iterate through all detected sensors and sync with map
	for (int i = 0; i<this->_sensorCount; i++){
		
		uint64_t id = this->transformId(sensorIds[i]);
		
		// add sensor if it is not yet in sensor map
		if(this->_mSensors.find(id) == this->_mSensors.end()) {
			
			this->_mSensors.insert(make_pair<uint64_t, DS18B20MeasurementBuffer>(id, DS18B20MeasurementBuffer()));
			_queue.call(this->_sensorAddedCallback, id);
		}
		
	}
	
	return OW_OK;
		
}

float DS18B20::getValue(uint64_t id) {
	
	if(this->_mSensors.find(id) == this->_mSensors.end()) {
		return DS18B20_INVALID_VALUE;
	} else {
		return this->_mSensors[id].get();
	}
	
}

OW_STATUS_CODE DS18B20::convertTemperature(void) {
	
	if (this->_oneWire->ow_reset() == OW_OK) {
		
		this->_oneWire->ow_command(DS18B20_COMMAND_CONVERT_T, NULL);
		this->_oneWire->ow_enable_strong_pullup();
		wait_ms(DS18B20_TCONV_12BIT); // todo timer with callback?
		this->_oneWire->ow_disable_strong_pullup();
		return OW_OK;
		
	} else { // no device present
		return OW_ERROR_NO_DEVICE;
	}
	
}

OW_STATUS_CODE DS18B20::readMeasurements(void) {
	
	for(map<uint64_t, DS18B20MeasurementBuffer>::const_iterator it = this->_mSensors.begin(); it != this->_mSensors.end(); it++) {
		
		char sp[DS18B20_SP_SIZE] = {};
		int reading;
		float result;
		char id[OW_ROMCODE_SIZE] = {};
		
		this->retrieveId(it->first, id);
		
		this->_oneWire->ow_reset();
		this->_oneWire->ow_command(DS18B20_COMMAND_READ_SCRATCHPAD, id);
		if (this->_oneWire->ow_read_bytes_with_crc_8(sp, DS18B20_SP_SIZE) != OW_OK) {
			(it->second).add(DS18B20_INVALID_VALUE);
		} else {
			reading = (sp[1] << 8) + sp[0];
	
			if (reading & 0x8000) { // negative degrees C
				reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
			}
	
			result = (float)reading / 16.0f;
			
			(it->second).add(result);
		}
	}
	
}

void DS18B20::collectMeasurement(void) {
	
	this->convertTemperature();
	this->readMeasurements();
	
}

uint64_t DS18B20::transformId(char romId[]) {
	
	return	(((uint64_t)romId[0]) << 56) | 
			(((uint64_t)romId[1]) << 48) | 
			(((uint64_t)romId[2]) << 40) |
			(((uint64_t)romId[3]) << 32) |
			(((uint64_t)romId[4]) << 24) |
			(((uint64_t)romId[5]) << 16) |
			(((uint64_t)romId[6]) << 8) |
			((uint64_t)romId[7]);
	
}

void DS18B20::retrieveId(uint64_t inId, char * outId) {
	
	for (int i=0; i<OW_ROMCODE_SIZE; i++) {
		outId[i] = (char) (inId >> (8 * (OW_ROMCODE_SIZE-1-i)));
	}
	
}