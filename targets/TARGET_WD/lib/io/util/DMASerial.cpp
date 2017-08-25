/*
 * DMASerial.cpp
 *
 * Created: 31.07.2017 11:02:43
 *  Author: Simon.Pfeifhofer
 */ 

#include "DMASerial.h"
extern DMA_HandleTypeDef DmaRxHandle[8];

DMASerial::DMASerial(PinName tx, PinName rx, int baud)
	: RawSerial(tx, rx, baud),
	consumer_pointer(0) { }

void DMASerial::popFrame(char * buffer, int * length, uint32_t timeout) {
	
//	if (this->_rx_cb) {
//		// cb is attached so we do not permit reads
//		*length = 0;
//		return;
//	}
	
	osEvent evt = _dma_frame_queue.get(timeout);
	
	if (evt.status == osEventMail) {
		
		dma_frame_meta_t * frame_meta = (dma_frame_meta_t *) evt.value.p;
		
		getFrame(frame_meta, buffer, length);
		
		_dma_frame_queue.free(frame_meta);
		
	} else {
		*length = 0;
		return;
	}
	
}

void DMASerial::getFrame(dma_frame_meta_t * frame_meta, char * buffer, int * length) {
	*length = frame_meta->frame_size;

	if (frame_meta->frame_start_pos + frame_meta->frame_size < frame_meta->buffer_size) {
		memcpy(buffer, frame_meta->buffer + frame_meta->frame_start_pos, frame_meta->frame_size);
	} else {
		int len = frame_meta->buffer_size - frame_meta->frame_start_pos;
		memcpy(buffer, frame_meta->buffer + frame_meta->frame_start_pos, len);
		memcpy(buffer + len, frame_meta->buffer, frame_meta->frame_size - len);
	}
}

void DMASerial::attachRxCallback(Callback<void(dma_frame_meta_t *)> func) {

	if (func){
		this->_rx_cb = func;
		this->_queueProcessingThread.start(mbed::Callback<void()>(this, &DMASerial::_process_queue_loop));
	}

}

void DMASerial::detachRxCallback(void) {
	
	this->_rx_cb = NULL;
	this->_queueProcessingThread.terminate();
	
}

int DMASerial::startRead(uint8_t *buffer, int buffer_size) {
	this->read(buffer, buffer_size, callback(this, &DMASerial::_dma_rx_capture), SERIAL_EVENT_RX_IDLE);
}

void DMASerial::_dma_rx_capture(int evt) {
	
    DMA_HandleTypeDef * hdma = &DmaRxHandle[this->_serial.serial.index];
	UART_HandleTypeDef * huart = (UART_HandleTypeDef *) hdma->Parent;
	uint16_t producer_pointer = huart->RxXferSize - __HAL_DMA_GET_COUNTER(hdma);
	
	dma_frame_meta_t * frame_meta = _dma_frame_queue.alloc();
	
	size_t frame_size;
	if (consumer_pointer < producer_pointer) {
		frame_size = producer_pointer - consumer_pointer;
		} else if (consumer_pointer > producer_pointer){
		frame_size = huart->RxXferSize - consumer_pointer + producer_pointer;
		} else {
		return;
	}
	
	if (frame_meta != NULL) {
		frame_meta->buffer = huart->pRxBuffPtr;
		frame_meta->buffer_size = huart->RxXferSize;
		frame_meta->frame_start_pos = consumer_pointer;
		frame_meta->frame_size = frame_size;
		if (_dma_frame_queue.put(frame_meta) != osOK) {
			wd_log_error("DMASerial: Unable to enqueue frame!");
		}
		} else {
		wd_log_error("DMASerial: Error allocating memory for frame queue!");
	}
	
	consumer_pointer = (consumer_pointer + frame_size) % huart->RxXferSize;
	
}

void DMASerial::_process_queue_loop(void) {
	
	while(true) {
		
		// we will wait here for frame reception
		osEvent evt = _dma_frame_queue.get(osWaitForever);
		
		if (evt.status == osEventMail) {
			
			dma_frame_meta_t * frame_meta = (dma_frame_meta_t *) evt.value.p;
			this->_rx_cb.call(frame_meta);
			_dma_frame_queue.free(frame_meta);
			
		}
		
	}

}