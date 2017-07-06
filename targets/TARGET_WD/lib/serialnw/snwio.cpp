/*
 * snwio.c
 *
 * Created: 04.07.2017 13:12:41
 *  Author: Guenter.Prossliner
 */ 

#include "snwio.h"
#include "snwconf.h"
#include "serial_api.h"
#include "lib_crc.h"

snwio::snwio(void)
{
	
}

void snwio::start()
{
	this->stats_reset();

}

int snwio::transmit_frame(void * data, size_t len)
{
	char * cdata = (char*)data;

	uint32_t crc = UINT32_MAX;
	for(int i=0; i<len;i++){
		// serial_putc()
		crc = update_crc_32(crc, cdata[i]);
	}

	
	union {
		uint32_t crc;
		char data[4];
	} crcdata = {crc};

	
}

snwio_stats snwio::stats_get(void)
{

}

snwio_stats snwio::stats_reset(void)
{
	snwio_stats s = this->m_stats;

	this->m_stats.rx_bytes = 0;
	this->m_stats.rx_frames_invalid = 0;
	this->m_stats.rx_frames_valid = 0;
	this->m_stats.tx_bytes = 0;
	this->m_stats.tx_frames = 0;

	return s;
}

 snwio::~snwio()
{

}

void snwio::handle_frame(void * data, size_t len)
{

}
