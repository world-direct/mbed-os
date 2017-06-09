#include "SensorDS18B20Temperature.h"

#define DS18B20_FAMILY_CODE		0x28

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

SensorDS18B20Temperature::SensorDS18B20Temperature(OneWire * oneWire) {
	
	this->_oneWire = oneWire;
	
}


SensorDS18B20Temperature::~SensorDS18B20Temperature() {}


OW_STATUS_CODE SensorDS18B20Temperature::convert_temperature(void) {
	
	if (this->_oneWire->ow_reset() == OW_OK) {
		
		this->_oneWire->ow_command(DS18B20_COMMAND_CONVERT_T, NULL);
		this->_oneWire->ow_enable_strong_pullup();
		wait_ms(DS18B20_TCONV_12BIT);
		this->_oneWire->ow_disable_strong_pullup();
		return OW_OK;
		
	} else { // no device present
		return OW_NO_DEVICE;
	}
	
}


float SensorDS18B20Temperature::getValue(void) {
	
	char sp[DS18B20_SP_SIZE];
	int reading;
	float result;
	
	this->convert_temperature();
	
	this->_oneWire->ow_reset();
	this->_oneWire->ow_command(DS18B20_COMMAND_READ_SCRATCHPAD, NULL);
	if (this->_oneWire->ow_read_bytes_with_crc_8(sp, DS18B20_SP_SIZE) != OW_OK)
		return INVALID_VALUE;
	
	reading = (sp[1] << 8) + sp[0];
	
	if (reading & 0x8000) { // negative degrees C
        reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
    }
	
    result = (float)reading / 16.0f;

	return result;
}
