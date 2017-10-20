/* 
* SerialModbus.h
*
* Created: 17.10.2017 09:09:47
* Author: Simon.Pfeifhofer
*/


#ifndef __SERIALMODBUS_H__
#define __SERIALMODBUS_H__

#define MODBUS_SERIAL_DEFAULT_STOPBITS	1
#define MODBUS_SERIAL_DEFAULT_PARITY	0
#define MODBUS_SERIAL_DEFAULT_BITS		8


#include "mbed.h"
#include "Modbus.h"

class SerialModbus : public Modbus
{
	//variables
	public:
	protected:
	private:
		Serial _serial;
		Mutex _mutex;
		Timer _timer;
		int _baud;
		SerialBase::Parity _parity;
		int _stopBits;
		
	//functions
	public:
		SerialModbus(PinName tx = RS485_Tx1, PinName rx = RS485_Rx1, int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE, int stopBits = MODBUS_SERIAL_DEFAULT_STOPBITS, SerialBase::Parity parity = (SerialBase::Parity)MODBUS_SERIAL_DEFAULT_PARITY, int bits = MODBUS_SERIAL_DEFAULT_BITS);
		~SerialModbus();
		uint8_t Read(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t* result_buffer);
		uint8_t Write(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t* write_buffer);
		int GetBaud();
		int GetStopBits();
		SerialBase::Parity GetParity();
		
	protected:
	private:
		SerialModbus( const SerialModbus &c );
		SerialModbus& operator=( const SerialModbus &c );
		Modbus::ModbusErrorCode write_request(uint8_t * request_datagram, size_t length);
		Modbus::ModbusErrorCode read_response(uint8_t * response_datagram, size_t length);
		bool serial_timeout_reached();
		uint8_t unlock_return(uint8_t return_code);
		static uint16_t calculate_CRC(uint8_t * buffer, int length);
		static void place_CRC(uint8_t * buffer, int payload_length);
		static bool check_CRC(uint8_t * buffer, int length);
		static uint8_t check_response(uint8_t * response_datagram, size_t length, uint8_t slave_id, uint8_t function_code);

}; //SerialModbus

#endif //__SERIALMODBUS_H__
