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
#include "ResettableTimeout.h"
//#include "platform/PlatformMutex.h"
//#include "platform/SingletonPtr.h"

#define BT_RX_BUFFER_SIZE	512
#define BT_TX_BUFFER_SIZE	512

extern DMA_HandleTypeDef DmaTxHandle[5];
extern DMA_HandleTypeDef DmaRxHandle[5];

class BusTransceiver : public RawSerial
{
//variables
public:
protected:
private:
	char * _bt_rx_buffer;
	char * _bt_tx_buffer;
	unsigned int _bt_rx_consumer;
	unsigned int _bt_rx_producer;
	
	InterruptIn * _bt_rx_pin;
	//ResettableTimeout * _bt_timeout;
	Ticker * _bt_timeout;
	
	static Mutex _bt_rx_mutex;
	static Mutex _bt_tx_mutex;

	EventQueue _queue;
	Thread _eventThread;
	
	
//functions
public:
	BusTransceiver(PinName Tx, PinName Rx, int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE);
	~BusTransceiver();
	
	void bt_start(void);
	void bt_transmit_frame(const void * data, size_t size);
	void bt_handle_frame(const void * data, size_t size);
	
protected:
private:
	BusTransceiver( const BusTransceiver &c );
	BusTransceiver& operator=( const BusTransceiver &c );

	void _bt_rx_active_interrupt(void);
	void _bt_rx_frame_received(void);
	void _bt_tx_complete(int evt);
	void _bt_rx_complete(int evt);
	float _bt_break_time_us(void) { return 18000000/_baud; };	// 1.5 frames = 18 bit

}; //BusTransceiver

#endif //__BUSTRANSCEIVER_H__
