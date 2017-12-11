/* 
* BTSlave.cpp
*
* Created: 29.11.2017 13:49:00
* Author: Patrick.Frischmann
*/

#include "BTSlave.h"

// default constructor
BTSlave::BTSlave(PinName Tx, PinName Rx, PinName Irq, PinName Led, int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE)
	:	BTBase(Tx, Rx, Irq, Led, baud),
		_irq(Irq, 1) {
	
	_state = BT_STATE_INITIAL;
	_tx_active_timeout = new ResettableTimeout(callback(this, &BTSlave::_on_tx_active_timeout), BT_SLAVE_MAX_TX_ACTIVE_WINDOW_MS * 1000);
	_tx_active_timeout->stop();
	
} //BTSlave

// default destructor
BTSlave::~BTSlave() {
	this->_txQueueProcessingThread.terminate();
} //~BTSlave

int BTSlave::_get_random_int(int from, int to) {
	return rand() % to + from;
	//std::default_random_engine generator;
	//std::uniform_int_distribution<int> distribution(from, to);
	//return distribution(generator);

}

void BTSlave::_tx_release(void) {

	_txQueueProcessingThread.signal_set(BT_SIG_TX_PROCESSING_RELEASE);
	
	// set timeout for tx active window
	_tx_active_timeout->reset();
	
}

void BTSlave::_tx_lock(void) {
	_txQueueProcessingThread.signal_clr(BT_SIG_TX_PROCESSING_RELEASE);
}

void BTSlave::_send_discover_ack(void) {

	// copy message type
	_tx_frame_buffer[0] = BT_MESSAGE_TYPE_DISCOVER;
	
	// copy id
	memcpy(_tx_frame_buffer + BT_FRAME_MESSAGE_TYPE_LENGTH, &_id, BT_FRAME_ADDRESS_LENGTH);

	_tx_buffer_flush(BT_FRAME_MESSAGE_TYPE_LENGTH + BT_FRAME_ADDRESS_LENGTH);

}

void BTSlave::_send_app_data_ack(void) {

	// copy message type
	_tx_frame_buffer[0] = BT_MESSAGE_TYPE_APPDATA_ACK;
	
	// copy id
	memcpy(_tx_frame_buffer + BT_FRAME_MESSAGE_TYPE_LENGTH, &_id, BT_FRAME_ADDRESS_LENGTH);

	_tx_buffer_flush(BT_FRAME_MESSAGE_TYPE_LENGTH + BT_FRAME_ADDRESS_LENGTH);

}

void BTSlave::_on_tx_active_timeout(void) {
	_tx_lock();
}

void BTSlave::_frame_received_internal(const char * data, size_t size) {
	
	// hint: frame is already considered valid by previous chain
	
	if(size < BT_FRAME_MESSAGE_TYPE_LENGTH + BT_FRAME_ADDRESS_LENGTH) {
		wd_log_error("BTSlave::_frame_received_internal() -> received invalid frame (length %d)", size);
		return;
	}
	
	// extract address
	uint64_t address = _get_address(data);
	
	switch(_get_message_type(data)) {
		
		case BT_MESSAGE_TYPE_DISCOVER:
			
			wd_log_info("BTSlave::_frame_received_internal() -> received discover message");
		
			if (address == BT_FRAME_ADDRESS_BROADCAST) {
			
				wd_log_debug("BTSlave::_frame_received_internal() -> discover message is broadcast");
				
				_state = BT_STATE_DISCOVER;
				_activity_led_off();
				
				// stop tx queue processing and empty queue
				_tx_lock();
				_tx_frame_queue_clear();

				// set bus interrupt to signal that we are here
				_bus_irq_set();
				
				// wait random time interval
				wait_ms(_get_random_int(1, BT_DISCOVER_LISTENING_PERIOD_MS - 50));
				
				// respond by submitting slave id
				_send_discover_ack();
				
			} else {
			
				wd_log_debug("BTSlave::_frame_received_internal() -> discover message is addressed to other party");
			
			}
		
			break;
		
		case BT_MESSAGE_TYPE_DISCOVER_FIN:
		
			wd_log_debug("BTSlave::_frame_received_internal() -> received discover fin message");
		
			if (address == _id) {
			
				wd_log_debug("BTSlave::_frame_received_internal() -> received discover fin message, switch to ready state");
				
				// received discover finish, clear interrupt line as we are registered
				_bus_irq_clr();
				
				_state = BT_STATE_READY;
				_activity_led_on();
				
				// discovery finished, inform upper layer
				_discoveryFinishedCallback.call();
				
			} else {
			
				wd_log_debug("BTSlave::_frame_received_internal() -> discover fin message is addressed to other party");
			
			}
		
			break;
		
		case BT_MESSAGE_TYPE_APPDATA:
		
			wd_log_info("BTSlave::_frame_received_internal() -> received app data message");
		
			if (address == _id){
			
				wd_log_debug("BTSlave::_frame_received_internal() -> app data message is addressed to me");
				
				// if payload is included, forward frame to upper layer for further processing, thereby skipping message type and address
				if(size > BT_FRAME_MESSAGE_TYPE_LENGTH + BT_FRAME_ADDRESS_LENGTH) {
					bt_handle_frame(data + BT_FRAME_MESSAGE_TYPE_LENGTH + BT_FRAME_ADDRESS_LENGTH, size - BT_FRAME_MESSAGE_TYPE_LENGTH - BT_FRAME_ADDRESS_LENGTH);
				}
				
				// allow tx processing
				_tx_release();
			
			} else {
				
				wd_log_debug("BTSlave::_frame_received_internal() -> app data message is addressed to other party");
					
			}
		
			break;
		
		case BT_MESSAGE_TYPE_APPDATA_ACK:
		
			wd_log_debug("BTSlave::_frame_received_internal() -> received app data ack message of other party");
			break;
		
		default:
			
			wd_log_error("BTSlave::_frame_received_internal() -> unknown message type!");
			
			break;
		
	}

}

void BTSlave::_start(void) {

	// set bus interrupt to signal that we are here
	_bus_irq_set();
	
	// start tx processing
	this->_txQueueProcessingThread.start(mbed::Callback<void()>(this, &BTSlave::_tx_queue_process_loop));

}

void BTSlave::_tx_queue_process_loop(void) {

	while(true) {
		
		Thread::signal_wait(BT_SIG_TX_PROCESSING_RELEASE);
		
//		wd_log_error("BTSlave::_tx_queue_process_loop() -> entry");
		
		// we will wait here for frame reception
		osEvent evt = _dma_tx_frame_queue.get(BT_SLAVE_TX_FRAME_QUEUE_TIMEOUT_MS);
		
		if (evt.status == osEventMail) {
			
			dma_frame_meta_t * frame_meta = (dma_frame_meta_t *) evt.value.p;
			
			size_t size;
			this->_dmaSerial->getFrame(frame_meta, this->_tx_frame_buffer, &size);
			
			_dma_tx_frame_queue.free(frame_meta);
			
			wd_log_debug("BTSlave::_tx_queue_process_loop() -> send app data");
			
			_tx_buffer_flush(size);
			
		} else {
			wd_log_debug("BTSlave::_tx_queue_process_loop() -> send ack");
			
			// nothing to send in queue (timeout reached), just send ack and stop tx processing
			_tx_active_timeout->stop();
			_tx_lock();
			
			_send_app_data_ack();
			
		}
		
	}

}

// implement this in the upper layer to handle valid frames
__weak void BTSlave::bt_handle_frame(const void * data, size_t size) {}