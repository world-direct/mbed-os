/*
 * DMASerial.cpp
 *
 * Created: 31.07.2017 11:02:43
 *  Author: Simon.Pfeifhofer
 */ 

#include "DMASerial.h"
extern DMA_HandleTypeDef DmaRxHandle[8];
static Mail<dma_frame_meta_t, DMASERIAL_RX_QUEUE_SIZE> _dma_frame_queue;
static int consumer_pointer = 0;

static void _dma_rx_capture(UART_HandleTypeDef * huart, uint8_t * data, uint16_t producer_pointer) {
	
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
		frame_meta->buffer = data;
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

void HAL_UART_RxIdleCallback(UART_HandleTypeDef *huart) {
	
	uint16_t producer_pointer = 0;
	if(huart->hdmarx != NULL)
	{
		DMA_HandleTypeDef *hdma = huart->hdmarx;
		
		//HAL_DMA_Abort(huart->hdmarx);
		
		/* Determine size/amount of received data */
		producer_pointer = huart->RxXferSize - __HAL_DMA_GET_COUNTER(hdma);
		
		huart->RxXferCount = 0;
		/* Check if a transmit process is ongoing or not */
		
		#if TARGET_WD_CORE
		
		huart->RxState = HAL_UART_STATE_READY;
	
		#else

		if(huart->State == HAL_UART_STATE_BUSY_TX_RX)
		{
			huart->State = HAL_UART_STATE_BUSY_TX;
		}
		else
		{
			huart->State = HAL_UART_STATE_READY;
		}

		#endif
		
	}
	
	_dma_rx_capture(huart, huart->pRxBuffPtr, producer_pointer);
	
}

int DMASerial::GetLength()
{
	DMA_HandleTypeDef *serial_handle = &DmaRxHandle[this->_serial.serial.index];
	return __HAL_DMA_GET_COUNTER(serial_handle);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	
	wd_log_warn("DMASerial: Rx complete callback (buffer is full -> rollover if circular)!");
	
//	// DMA buffer full...
//	HAL_DMA_Abort(huart->hdmarx);
//	
//	// disable idle interrupt (otherwise it would be called as well)
//	__HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
//	
//	// chances are this won't be a valid frame, however we need to clean the buffer
//	_dma_rx_capture(huart, huart->pRxBuffPtr, huart->RxXferSize);
//	
//	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	
}

DMASerial::DMASerial(PinName tx, PinName rx, int baud)
: RawSerial(tx, rx, baud) { }

void DMASerial::popFrame(char * buffer, int * length, uint32_t timeout) {
	
	if (this->_rx_cb) {
		// cb is attached so we do not permit reads
		*length = 0;
		return;
	}
	
	osEvent evt = _dma_frame_queue.get(timeout);
	
	if (evt.status == osEventMail) {
		
		dma_frame_meta_t * frame_meta = (dma_frame_meta_t *) evt.value.p;
		
		*length = frame_meta->frame_size;
		
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