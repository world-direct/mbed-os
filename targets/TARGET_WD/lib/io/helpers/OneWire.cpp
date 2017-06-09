#include "OneWire.h"
#include "platform/critical.h"

#define OW_DELAY_US(value) wait_us(value)

OneWire::OneWire(PinName pinRx, PinName pinTx, PinName pinTxH)
	: _pinRx(pinRx), _pinTx(pinTx, PIN_OUTPUT, OpenDrain, 0) {}

OneWire::~OneWire() {}

void OneWire::ow_write_bit(bool b) {
	
	this->ow_bus_low();
	
	if (b) {
		OW_DELAY_US(6);
		this->ow_bus_high();
		OW_DELAY_US(64);
	} else {
		OW_DELAY_US(60);
		this->ow_bus_high();
		OW_DELAY_US(10);
	}
	
}

bool OneWire::ow_read_bit(void) {
	
	bool response;
	
	this->ow_bus_low();
	OW_DELAY_US(6);
	this->ow_bus_high();
	OW_DELAY_US(9);
	response = _pinRx.read();
	OW_DELAY_US(55);
	
	return response;
	
}

char OneWire::ow_crc_8(char * data, size_t length) {
	
	char crc = 0x00;
	
	while (length--) {
		char inbyte = *data++;
		for (int i = 8; i; i--) {
			char mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	
	return crc;
	
}

OW_STATUS_CODE OneWire::ow_reset(void) {
	/*
		Drive bus low, delay 480us
		Release bus, delay 70us
		Sample bus, 0 = device(s) present, 1 = no device present
		Delay 410us
	*/
	
	bool ow_device_present = false;
	
	core_util_critical_section_enter();
	
	this->ow_bus_low();
	OW_DELAY_US(480);
	this->ow_bus_high();
	
	OW_DELAY_US(70);
	
	if (_pinRx.read() == 0)
		ow_device_present = true;
	
	OW_DELAY_US(410);
	
	core_util_critical_section_exit();
	
	return ow_device_present ? OW_OK : OW_ERROR_CRC;
	
}

OW_STATUS_CODE OneWire::ow_write_byte(char data) {
	
	core_util_critical_section_enter();
    for (int i = 0; i < 8; i++) {
        this->ow_write_bit((data >> i) & 0x01);
    }
	core_util_critical_section_exit();
	
	return OW_OK;
	
}

OW_STATUS_CODE OneWire::ow_read_byte(char * buffer) {
	
	*buffer = 0x00;
	
	core_util_critical_section_enter();
	for (int i = 0; i < 8; i++) {
		bool bit = this->ow_read_bit();
		if(bit) (*buffer) |= (0x01 << i);
	}
	core_util_critical_section_exit();
	
	return OW_OK;
	
}

OW_STATUS_CODE OneWire::ow_read_bytes_with_crc_8(char * buffer, size_t length) {
	
	for(int i = 0; i < length; i++){
		this->ow_read_byte(buffer + i);
	}
	
	char crcRead;
	this->ow_read_byte(&crcRead);
	char crcCalculated = this->ow_crc_8(buffer, length);
	
	return crcRead == crcCalculated ? OW_OK : OW_ERROR_CRC;
	
}

OW_STATUS_CODE OneWire::ow_command(char command, char id[]) {
	
	if (id) { // write to specific device
		this->ow_write_byte(OW_MATCH_ROM);
		int i = OW_ROMCODE_SIZE;	
		do {
			this->ow_write_byte(*id);
			++id;
		} while (--i);
	} else { // write to all devices
		this->ow_write_byte(OW_SKIP_ROM);
	}
	
	this->ow_write_byte(command);
	
	return OW_OK;
	
}