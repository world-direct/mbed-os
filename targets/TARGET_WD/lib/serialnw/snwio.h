/*
 * snwio.h
 *
 * Created: 04.07.2017 13:13:06
 *  Author: Guenter.Prossliner
 */ 


#ifndef SNWIO_H_
#define SNWIO_H_

#include <stddef.h>

struct snwio_stats {

	int rx_bytes;
	int tx_bytes;

	int tx_frames;
	int rx_frames_valid;
	int rx_frames_invalid;
};

class snwio {

	public:
		snwio(void);
		void start();
		int transmit_frame(void * data, size_t len);
		snwio_stats stats_get(void);
		snwio_stats stats_reset(void);

	protected:
		virtual ~snwio();
		virtual void handle_frame(void * data, size_t len);

	private:
		snwio_stats m_stats;
};


#endif /* SNWIO_H_ */