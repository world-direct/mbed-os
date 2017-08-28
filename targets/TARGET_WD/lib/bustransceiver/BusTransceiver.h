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

#define BT_BUFFER_SIZE			DMASERIAL_RX_BUFFER_SIZE
#define BT_TX_WRITE_TIMEOUT		100
#define BT_TX_ECHO_TIMEOUT		100

class BusTransceiver {
	
//variables
public:
protected:
private:
	
	char * _bt_rx_buffer;
	char * _bt_tx_buffer;
	
	DMASerial *_dmaSerial;
	
	rtos::Semaphore _tx_complete_sem;
	rtos::Semaphore _tx_echo_received_sem;
	
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

	void _bt_rx_process_frame(dma_frame_meta_t * frame_meta);
	void _bt_tx_complete(int evt);

}; //BusTransceiver

#endif //__BUSTRANSCEIVER_H__
