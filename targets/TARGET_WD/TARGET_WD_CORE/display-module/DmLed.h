/*
 * DmLed.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created:	12.05.2017
 * Author:	Patrick Frischmann
 * EMail:	patrick.frischmann@world-direct.at
 *
 * Description:
 *        Enables LED control of WD Display Module.
 *
 */ 

#ifndef DMLED_H
#define DMLED_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/
// Colors
#define LED_COLOR_RED				0xFF0000
#define LED_COLOR_GREEN				0x00FF00
#define LED_COLOR_BLUE				0x0000FF
#define LED_COLOR_BLACK				0x000000
#define LED_COLOR_WHITE				(LED_COLOR_RED | LED_COLOR_GREEN | LED_COLOR_BLUE)
#define LED_COLOR_YELLOW			(LED_COLOR_RED | LED_COLOR_GREEN)
#define LED_COLOR_MAGENTA			(LED_COLOR_RED | LED_COLOR_BLUE)
#define LED_COLOR_CYAN				(LED_COLOR_GREEN | LED_COLOR_BLUE)


/******************************************************
___________________DECLARATION_________________________
******************************************************/
class DmLed {
public:
	DmLed(PinName mosi, PinName sck, PinName cs);
	~DmLed();
	
	void led1_on(uint32_t color);
	void led1_off(void);
	void led2_on(uint32_t color);
	void led2_off(void);
	
private:
	void led_on(uint8_t led, uint32_t color);
	void led_off(uint8_t led);
	void apply(void);
	
	uint8_t _stateCache;
	DigitalOut* _pinCS;
	SPI* _spi;
};	

#endif // !DMLED_H