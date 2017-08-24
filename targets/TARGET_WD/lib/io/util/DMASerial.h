/*
 * DMASerial.h
 *
 * Created: 31.07.2017 11:01:37
 *  Author: Simon.Pfeifhofer
 */ 


#ifndef DMASERIAL_H_
#define DMASERIAL_H_

#include "mbed.h"
#include "device.h"
#include "rtos.h"
#include "wd_logging.h"

#define DMASERIAL_RX_BUFFER_SIZE	512
#define DMASERIAL_RX_QUEUE_SIZE		10

typedef struct {
	uint8_t * buffer;
	size_t buffer_size;
	unsigned int frame_start_pos;
	size_t frame_size;
} dma_frame_meta_t;

class DMASerial : public RawSerial{
    
public:
	
    DMASerial(PinName tx, PinName rx, int baud);
	
	void popFrame(char * buffer, int * length, uint32_t timeout = osWaitForever);
	void getFrame(dma_frame_meta_t * frame_meta, char * buffer, int * length);
	void attachRxCallback(Callback<void(dma_frame_meta_t *)> func);
	void detachRxCallback(void);
	int GetLength(void);

private:
	Thread _queueProcessingThread;
	Callback<void(dma_frame_meta_t *)> _rx_cb; 
	
	void _process_queue_loop(void);
};


#endif /* DMASERIAL_H_ */