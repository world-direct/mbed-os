/*
 * DMASerial.cpp
 *
 * Created: 31.07.2017 11:02:43
 *  Author: Simon.Pfeifhofer
 */ 

#include "DMASerial.h"
#include "mbed.h"

extern DMA_HandleTypeDef DmaRxHandle[8];

 DMASerial::DMASerial(PinName tx, PinName rx, int baud)
 : RawSerial(tx, rx, baud)
{
	
}

int DMASerial::GetLength()
{
	DMA_HandleTypeDef *serial_handle = &DmaRxHandle[this->_serial.serial.index];
	return __HAL_DMA_GET_COUNTER(serial_handle);
}
