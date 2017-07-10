/*
 * snwio.c
 *
 * Created: 04.07.2017 13:12:41
 *  Author: Guenter.Prossliner
 */ 

#include "snwio.h"
#include "snwconf.h"
#include "RawSerial.h"
#include "serial_api.h"

extern "C" {
	// there is no extern "C" in lib_crc.h, so we have to put it here
	// because I don't want to modify the .h file
	#include "lib_crc.h"
}

static uint8_t rx_buffer[1024];

snwio::snwio(PinName tx, PinName rx, int baud)
{
	serial_init(&this->m_serial, tx, rx);
	serial_baud(&this->m_serial, baud);

	// s.puts("Hallo Bus!");
	//s.read(rx_buffer, sizeof(rx_buffer), this->rxcallback, SERIAL_EVENT_RX_ALL)
}

void snwio::start()
{
	this->stats_reset();

	this->m_serial.rx_buff.buffer = rx_buffer;
	this->m_serial.rx_buff.length = sizeof(rx_buffer);

	static const char * HelloWorld = "Hello Bus!";
	this->transmit_frame(HelloWorld, sizeof(HelloWorld));

	this->p_initrx();

}

int snwio::transmit_frame(void * data, size_t len)
{
	char * cdata = (char*)data;

	uint32_t crc = UINT32_MAX;
	for(int i=0; i<len;i++){
		char c = cdata[i];
		serial_putc(&this->m_serial, c);
		crc = update_crc_32(crc, c);
	}

	
	union {
		uint32_t crc;
		char data[4];
	} crcdata = {crc};

	for(int i=0; i<4; i++){
		serial_putc(&this->m_serial, crcdata.data[i]);
	}
}

snwio_stats snwio::stats_get(void)
{
	return this->m_stats;
}

snwio_stats snwio::stats_reset(void)
{
	//snwio_stats s = this->m_stats;

	this->m_stats = {};

	//return s;
}

snwio::~snwio()
{

}

void snwio::handle_frame(void * data, size_t len)
{

}

void snwio::p_initrx()
{
	this->m_serial.rx_buff.pos = 0;
}

void snwio::p_tx(void * data, size_t len)
{
	
}
