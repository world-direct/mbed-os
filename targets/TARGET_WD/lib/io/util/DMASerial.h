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
#define DMASERIAL_RX_QUEUE_SIZE		5

// TODO: currently we need lots of memory here...
typedef struct {
	char data[DMASERIAL_RX_BUFFER_SIZE];
	size_t size;
} dma_frame_t;

class DMASerial : public RawSerial{
    
public:
	
    DMASerial(PinName tx, PinName rx, int baud);
	
	void getFrame(uint8_t * buffer, int * length, uint32_t timeout = osWaitForever);
	void attachRxCallback(Callback<void(dma_frame_t *)> func);
	void detachRxCallback(void);
	
private:
	Thread _queueProcessingThread;
	Callback<void(dma_frame_t *)> _rx_cb; 
	
	void _process_queue_loop(void);

};


#endif /* DMASERIAL_H_ */