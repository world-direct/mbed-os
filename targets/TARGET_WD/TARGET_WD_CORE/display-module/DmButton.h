/*
 * DmButton.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created:	12.05.2017
 * Author:	Patrick Frischmann
 * EMail:	patrick.frischmann@world-direct.at
 *
 * Description:
 *        Enables Button functionality for WD Display Module.
 *
 */ 

#ifndef DMBUTTON_H
#define DMBUTTON_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"
#include "platform/Callback.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/


/******************************************************
___________________DECLARATION_________________________
******************************************************/
class DmButton {
public:
	DmButton(PinName miso, PinName sck, PinName cs, PinName irq);
	~DmButton();
	
	void onButtonPressed(void);
	
	enum IrqSource {
        Button1 = 0,
        Button2,
        Button3,
        Button4,

        IrqSourceCnt
    };
	
	void attach(Callback<void()> func, IrqSource type=Button1);
	
private:
	InterruptIn _pinIRQ;
	DigitalOut _pinCS;
	SPI _spi;
	Callback<void()> _irq[IrqSourceCnt];
};	

#endif // !DMBUTTON_H