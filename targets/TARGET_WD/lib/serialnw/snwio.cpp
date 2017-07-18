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
#include "snwconf.h"
#include "us_ticker_api.h"

extern "C" {
	// there is no extern "C" in lib_crc.h, so we have to put it here
	// because I don't want to modify the .h file
	#include "lib_crc.h"
}


static serial_t m_serial;
static char	m_rx_buffer[SNWIO_CONF_RX_BUFFER_SIZE];
static char m_tx_buffer[SNWIO_CONF_TX_BUFFER_SIZE];
static size_t m_tx_size;
static int m_us_pause_time;
static snwio_stats m_stats;

void snwio_init()
{
	serial_init(&m_serial, SNWIO_CONF_PIN_TX, SNWIO_CONF_PIN_RX);
	serial_baud(&m_serial, SNWIO_CONF_BAUD);

	m_us_pause_time = SystemCoreClock / 
		(SNWIO_CONF_BAUD * 10); // 8 Data, 1 Start, 1 Stop-Bit 
}

// this is just a declaration, this method has to be implemented in the upper layer
void snwio_frame_received(void * data, size_t size);

typedef struct {
	char value;
	struct {
		int timeout_occured : 1;
	};
} _snwio_char_t;

// returns char 0-FF, or -1 if timeout
static _snwio_char_t _snwio_readchar()
{
	uint32_t start_time = us_ticker_read();
	uint32_t end_time = start_time + m_us_pause_time;

	_snwio_char_t ret;
	ret.timeout_occured = 0;

	while(us_ticker_read() - start_time > m_us_pause_time){
		if(serial_readable(&m_serial)){
			m_stats.rx_bytes++;
			char c = (char)serial_getc(&m_serial);
			ret.value = c;
			return ret;
		}
	};

	ret.timeout_occured = 1;
	return ret;
}

static _snwio_char_t _snwio_writechar(char c){
	serial_putc(&m_serial, c);
	_snwio_char_t echoc = _snwio_readchar();
	if(echoc.timeout_occured){
		// THIS IS A FATAL ERROR, WE SHOULD ALWAYS READ OUR CHAR!
		return;
	}

	return echoc;
}

static void _snwio_readframe()
{
	uint32_t crc = UINT32_MAX;

	int i = 0;
	for(;i<SNWIO_CONF_RX_BUFFER_SIZE; i++){
		_snwio_char_t c =  _snwio_readchar();
		if(c.timeout_occured) {
			// done with frame!
			break;
		}
		crc = update_crc_32(crc, c.value);

		m_rx_buffer[i] = c.value;
	}

	if(i == SNWIO_CONF_RX_BUFFER_SIZE){
		// OVERRUN!
		m_stats.rx_overruns++;
		// let's read the rest, so that we may handle the next frame
		while(!_snwio_readchar().timeout_occured);
		return;
	}

	// we have the frame, let's check the CRC
	if(crc == 0){
		// OK!!!, let it get handled by the upper layer, but we will hide the CRC from the size
		m_stats.rx_frames_valid++;
		snwio_frame_received(m_rx_buffer, i - 4);
	} else {
		// CRC Error!!!
		m_stats.rx_frames_invalid++;
	}
	
	
}

static void _snwio_writeframe()
{
	uint32_t crc = UINT32_MAX;

	for(int i=0; i<m_tx_size; i++){
		char c = m_tx_buffer[i];
		if(_snwio_writechar(c).value != c){
			// collision, may exit early!
		}
		crc = update_crc_32(crc, c);
	}

	// write the crc
	union {
		uint32_t crc;
		char data[4];
	} crcdata = {crc};

	for(int i=0; i<3; i++){
		_snwio_writechar(crcdata.data[i]);
	}
	
	// let's wait sync the pause chars, we also could wait "optimistic", but this would required to handle overflow of sysclk
	wait_us(m_us_pause_time);
}

void snwio_transfer_frame(const void * data, size_t size)
{
	if(size > SNWIO_CONF_TX_BUFFER_SIZE){
		// OVERFLOW!
		return;
	}

	memcpy(m_tx_buffer, data, size);
	m_tx_size = size;	// this will trigger tx in next loop
}

void snwio_loop_check()
{
	// RX started from other peer?
	if(serial_readable(&m_serial)){
		_snwio_readframe();
	}

	// start tx?
	if(m_tx_size > 0){
		_snwio_writeframe();
	}
}
