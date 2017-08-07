/*
 * DMASerial.h
 *
 * Created: 31.07.2017 11:01:37
 *  Author: Simon.Pfeifhofer
 */ 


#ifndef DMASERIAL_H_
#define DMASERIAL_H_

#include "mbed.h"

class DMASerial : public RawSerial{
    
public:
    
    DMASerial(PinName tx, PinName rx, int baud);
	
	int GetLength();
	
};


#endif /* DMASERIAL_H_ */