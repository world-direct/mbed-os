/*
 * DMASerial.cpp
 *
 * Created: 31.07.2017 11:02:43
 *  Author: Simon.Pfeifhofer
 */ 

#include "DMASerial.h"

static Mail<dma_frame_t, DMASERIAL_RX_QUEUE_SIZE> _dma_frame_queue;

static void default_handle_frame(dma_frame_t * frame) {}
	
static void _dma_rx_capture(UART_HandleTypeDef * huart, uint8_t * data, uint16_t size) {
	
	/*
	 * TODO:
	 * avoid performance bottleneck by replacing copy of DMA
	 * buffer with buffer switching as DMA is paused during
	 * copy operation!
	 */
	
	// copy and enqueue received data
	dma_frame_t * dma_frame = _dma_frame_queue.alloc();
	memcpy(dma_frame->data, data, size);
	dma_frame->size = size;
	_dma_frame_queue.put(dma_frame);
	
	// restart DMA transfer
	HAL_UART_Receive_DMA(huart, huart->pRxBuffPtr, huart->RxXferSize);
	
}

void HAL_UART_RxIdleCallback(UART_HandleTypeDef *huart) {
	
	uint16_t rxXferCount = 0;
	if(huart->hdmarx != NULL)
	{
		DMA_HandleTypeDef *hdma = huart->hdmarx;
		
		/* Determine size/amount of received data */
		rxXferCount = huart->RxXferSize - __HAL_DMA_GET_COUNTER(hdma);
		
		HAL_DMA_Abort(huart->hdmarx);
		
		huart->RxXferCount = 0;
		/* Check if a transmit process is ongoing or not */
		if(huart->State == HAL_UART_STATE_BUSY_TX_RX)
		{
			huart->State = HAL_UART_STATE_BUSY_TX;
		}
		else
		{
			huart->State = HAL_UART_STATE_READY;
		}
		
	}
	
	_dma_rx_capture(huart, huart->pRxBuffPtr, rxXferCount);
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	
	// DMA buffer full...
	
	// disable idle interrupt (otherwise it would be called as well)
	__HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
	
	// chances are this won't be a valid frame, however we need to clean the buffer
	_dma_rx_capture(huart, huart->pRxBuffPtr, huart->RxXferSize);
	
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	
}

DMASerial::DMASerial(PinName tx, PinName rx, int baud)
: RawSerial(tx, rx, baud) {
	
	this->_rx_cb = default_handle_frame;
	this->_queueProcessingThread.start(mbed::Callback<void()>(this, &DMASerial::_process_queue_loop));
}

void DMASerial::attachRxCallback(Callback<void(dma_frame_t *)> func) {

	if (func){
		this->_rx_cb = func;
	} else {
		this->_rx_cb = default_handle_frame;
	}

}

void DMASerial::_process_queue_loop(void) {
	
	while(true) {
		
		// we will wait here for frame reception
		osEvent evt = _dma_frame_queue.get();
		
		if (evt.status == osEventMail) {
			
			dma_frame_t * dma_frame = (dma_frame_t *) evt.value.p;
			this->_rx_cb.call(dma_frame);
			_dma_frame_queue.free(dma_frame);
			
		}
		
	}

}