/* 
* TCPModbus.cpp
*
* Created: 04.12.2017 14:04:08
* Author: Simon.Pfeifhofer
*/


#include "TCPModbus.h"

// default constructor
TCPModbus::TCPModbus(EthernetInterface * ethernetInterface) : _socket(ethernetInterface)
{
} //TCPModbus

// default destructor
TCPModbus::~TCPModbus()
{
} //~TCPModbus

uint8_t TCPModbus::Read(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t* result_buffer){
	
	if(_socket.connect(_slaveAddress) != 0){
		return Modbus::TCPConnect;
	}
	
	// Remark: 
	// Modbus requires big-endianess on the wire (http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf)
	// The used ARM Cortex-microcontrollers uses big-endianess
	// Therefor we can leave the bytes in the same order
	
	uint8_t request[
		2 + // MBAP-Header:		Transaction-Identifier
		2 + // MBAP-Header:		Protocol-Identifier
		2 + // MBAP-Header:		Length
		1 + // MBAP-Header:		Unit-Identifier
		1 + // MODBUS-PDU:		Function code
		2 + // MODBUS-PDU:		Starting address
		2	// MODBUS-PDU:		Quantity of Registers
	];
	
	// MBAP-Header: Transaction-Identifier
	uint16_t transaction_identifier = 0;
	memcpy(request, &transaction_identifier, sizeof(uint16_t));
	
	// MBAP-Header: Protocol-Identifier
	uint16_t protocol_identifier = 0;
	memcpy(request + sizeof(uint16_t), &protocol_identifier, sizeof(uint16_t));
	
	// MBAP-Header: Length
	uint16_t length = 6;
	memcpy(request + 2 * sizeof(uint16_t), &length, sizeof(uint16_t));
	
	// MBAP-Header:	Unit-Identifier
	request[3*sizeof(uint16_t)] = slave_id;
	
	// MODBUS-PDU: Function code (read holding registers)
	uint8_t function_code = 0x03;
	request[3*sizeof(uint16_t) + 1] = function_code;
	
	// MODBUS-PDU: Starting address
	memcpy(request + 4 * sizeof(uint16_t), &start_address, sizeof(uint16_t));
	
	// MODBUS-PDU: Quantity of Registers
	memcpy(request + 5 * sizeof(uint16_t), &register_count, sizeof(uint16_t));
	
	// Send request
	if(_socket.send(request, sizeof(request)) < 0){
		return this->TCPClose(Modbus::TCPSend);
	}
	
	uint8_t response[
		sizeof(uint16_t) +									// MBAP-Header:		Transaction-Identifier
		sizeof(uint16_t) +									// MBAP-Header:		Protocol-Identifier
		sizeof(uint16_t) +									// MBAP-Header:		Length
		sizeof(uint8_t) +									// MBAP-Header:		Unit-Identifier
		sizeof(uint8_t) +									// MODBUS-PDU:		Function code
		register_count * sizeof(uint16_t)	// MODBUS-PDU:		Registers
	];
	
	// Read
	uint16_t real_size = 0;	
	if((real_size = _socket.recv(response, sizeof(response))) < 0){
		return this->TCPClose(Modbus::TCPReceive);
	}
	
	// Check length
	if(real_size != sizeof(response)){
		return this->TCPClose(Modbus::TCPLengthRawRead);
	}
	
	// Check Transaction-Identifier
	if(memcmp(response, request, sizeof(uint16_t)) != 0){
		return this->TCPClose(Modbus::TCPTransactionIdentifier);
	}
	
	// Check Protocol-Identifier
	if(memcmp(response + sizeof(uint16_t), request + sizeof(uint16_t), sizeof(uint16_t)) != 0){
		return this->TCPClose(Modbus::TCPProtocolIdentifier);
	}
	
	// Check Length
	uint16_t response_payload_length;
	memcpy(&response_payload_length, response + 2*sizeof(uint16_t), sizeof(uint16_t));
	if(response_payload_length != (register_count * sizeof(uint16_t) + sizeof(uint16_t))){
		return this->TCPClose(Modbus::TCPLengthPayload);
	}
	
	// Check Unit-Identifier
	if(response[6] != slave_id){
		return this->TCPClose(Modbus::SlaveId);
	}
	
	// Check Function-Code
	if(response[7] != function_code){
		return this->TCPClose(response[7]);
	}
	
	// Copy payload
	memcpy(result_buffer, response + 4*sizeof(uint16_t), sizeof(uint16_t) * register_count);
	
	return this->TCPClose();
	
}

uint8_t TCPModbus::Write(uint8_t slave_id, uint16_t start_address, uint16_t register_count, uint8_t * write_buffer){
	return Modbus::Success;
}

char* TCPModbus::GetIpAddress(){
	return _slaveAddress.get_ip_address();
}

void TCPModbus::SetIpAddress(char * ip_address, int length){
	char ip_address_terminated[length];
	memcpy(ip_address_terminated, ip_address, length);
	ip_address_terminated[length] = '\0';
	_slaveAddress.set_ip_address(ip_address_terminated);
}

int TCPModbus::GetPort(){
	return _slaveAddress.get_port();
}

void TCPModbus::SetPort(int port){
	_slaveAddress.set_port(port);
}

uint8_t TCPModbus::TCPClose(ModbusErrorCode error_code){
	
	if(_socket.close() != 0){
		return Modbus::TCPClose;
	}
	return error_code;
	
}
