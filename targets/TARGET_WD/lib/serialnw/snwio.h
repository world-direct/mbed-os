/*
 * snwio.h
 *
 * Created: 04.07.2017 13:13:06
 *  Author: Guenter.Prossliner
 */ 


#ifndef SNWIO_H_
#define SNWIO_H_

#include <stddef.h>
#include "PinNames.h"

// includes for private: members
#include "mbed.h"

#define RX_BUFFER_SIZE	64

struct snwio_stats {

	int rx_overruns;

	int rx_bytes;
	int tx_bytes;

	int tx_frames;
	int rx_frames_valid;
	int rx_frames_invalid;
};

class snwio {

	public:
		snwio(PinName tx, PinName rx, int baud);

		void start();
		int transmit_frame(void * data, size_t len);
		snwio_stats stats_get(void);
		snwio_stats stats_reset(void);
		virtual ~snwio();

	protected:
		virtual void handle_frame(void * data, size_t len);

	private:
		uint8_t m_rxbuffer[RX_BUFFER_SIZE];
		int m_rxbuffer_offset;
		snwio_stats m_stats;
		RawSerial m_serial;
		void p_rxIrq(void);
		void p_rx(void);
		void p_tx(void * data, size_t len);
};


#endif /* SNWIO_H_ */