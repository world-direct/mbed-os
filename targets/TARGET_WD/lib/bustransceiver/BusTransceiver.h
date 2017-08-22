/* 
* BusTransceiver.h
*
* Created: 28.07.2017 09:59:39
* Author: Patrick.Frischmann
*/


#ifndef __BUSTRANSCEIVER_H__
#define __BUSTRANSCEIVER_H__

#include "mbed.h"
#include "objects.h"
#include "DMASerial.h"
#include "Mutex.h"

#define BT_BUFFER_SIZE	512
#define BT_EOF_CHAR_MATCH			(126)
#define BT_RX_CHECK_INTERVAL_MSEC	100

#define BT_RX_READ_TIMEOUT1			10
#define BT_RX_READ_TIMEOUT2			100
#define BT_TX_WRITE_TIMEOUT			100

extern DMA_HandleTypeDef DmaTxHandle[5];
extern DMA_HandleTypeDef DmaRxHandle[5];

class BusTransceiver {
	
//variables
public:
protected:
private:
	
	char * _bt_rx_buffer;
	char * _bt_tx_buffer;
	
	char * _bt_rx_buffer_step;
	
	unsigned int _bt_rx_consumer;
	unsigned int _bt_rx_producer;
	
	DMASerial *_dmaSerial;
	
	rtos::Mutex _mutex;
	rtos::Semaphore _tx_semaphore;
	Timer _stepTimer;
	Thread _readProcessingThread;
	
//functions
public:
	BusTransceiver(PinName Tx, PinName Rx, int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE);
	~BusTransceiver();
	
	void bt_start(void);
	void bt_transmit_frame(const void * data, size_t size);
	void bt_handle_frame(const void * data, size_t size);
	
protected:
private:
	BusTransceiver(const BusTransceiver &c);
	BusTransceiver& operator=(const BusTransceiver &c);

	void _bt_rx_entry(void);
	void _bt_rx_step(void);
	void _bt_rx_locked_step(void);
	void _bt_tx_complete(int evt);
	void _bt_rx_complete(int evt);

}; //BusTransceiver

#endif //__BUSTRANSCEIVER_H__
