/* 
* BTMaster.h
*
* Created: 29.11.2017 13:48:48
* Author: Patrick.Frischmann
*/


#ifndef __BTMASTER_H__
#define __BTMASTER_H__

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "BTBase.h"
#include <iterator>
#include "ResettableTimeout.h"
#include "BTSlaveCollection.h"

/******************************************************
___________________DEFINES_____________________________
******************************************************/

#define BT_MASTER_DISCOVER_FIN_TIMEOUT_MS			500
#define BT_MASTER_MESSAGE_ACK_TIMEOUT_MS			50
#define BT_MASTER_MIN_COMMUNICATION_INTERVAL_MS		100
#define BT_MASTER_TX_QUEUE_LOCK_TIMEOUT_MS			100

class BTMaster : public BTBase
{
//variables
public:
protected:
private:
	InterruptIn _irq;
	rtos::Thread _mainLoopThread;
	ResettableTimeout * _txQueueLockTimeout;
	BTSlaveCollection _slaveCollection;
	
//functions
public:
	BTMaster(PinName Tx, PinName Rx, PinName Irq, PinName Led, int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE);
	~BTMaster();
	void bt_handle_frame(uint64_t sender, const void * data, size_t size);
	int bt_get_slave_count(void) { return _slaveCollection.getCount(); };
	void bt_get_slave_ids(uint64_t * buffer) { return _slaveCollection.getIds(buffer); };
	
protected:
	void _start(void);
	void _frame_received_internal(const char * data, size_t size);
	

private:
	BTMaster( const BTMaster &c );
	BTMaster& operator=( const BTMaster &c );
	
	void _main_loop(void);
	
	void _on_bus_irq(void);
	void _send_discover_broadcast(void);
	void _send_discover_fin(void);
	void _send_select_slave(void);
	void _tx_release(void);
	void _tx_lock(void);

}; //BTMaster

#endif //__BTMASTER_H__
