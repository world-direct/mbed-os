/* 
* SerialModbus.h
*
* Created: 17.10.2017 09:09:47
* Author: Simon.Pfeifhofer
*/


#ifndef __SERIALMODBUS_H__
#define __SERIALMODBUS_H__

#include "mbed.h"
#include "Modbus.h"

class SerialModbus : public Modbus
{
	//variables
	public:
	protected:
	private:
		Serial _serial;

	//functions
	public:
		SerialModbus(PinName tx, PinName rx, int baud);
		~SerialModbus();
		uint8_t Read(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t* result_buffer);
		uint8_t Write(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t* write_buffer);
	protected:
	private:
		SerialModbus( const SerialModbus &c );
		SerialModbus& operator=( const SerialModbus &c );
		static uint16_t calculate_CRC(uint8_t * buffer, int length);
		static void place_CRC(uint8_t * buffer, int payload_length);
		static bool check_CRC(uint8_t * buffer, int length);
		static uint8_t check_response(uint8_t * response_datagram, size_t length, uint8_t slave_id);

}; //SerialModbus

#endif //__SERIALMODBUS_H__
