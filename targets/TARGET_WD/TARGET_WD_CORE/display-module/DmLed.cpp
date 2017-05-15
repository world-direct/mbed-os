/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "DmLed.h"
#include "BitIO.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/
#define LED1	0x0
#define LED2	0x1

#define LED1_GREEN_BIT		0x0
#define LED1_BLUE_BIT		0x1
#define LED1_RED_BIT		0x2
#define LED2_GREEN_BIT		0x3
#define LED2_BLUE_BIT		0x4
#define LED2_RED_BIT		0x5


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/
DmLed::DmLed(PinName mosi, PinName sck, PinName cs) {
	_pinCS = new DigitalOut(cs, 0);	// store pin
	_spi = new SPI(mosi, NC, sck);
	_spi->format(8, 2);
//	_spi->frequency(115200);
	
	_stateCache = 0xFF;
	led1_off();
	led2_off();
}

DmLed::~DmLed() {
	delete _pinCS;
	delete _spi;
	
	_pinCS = NULL;
	_spi = NULL;
}

void DmLed::apply(void) {
	_pinCS->write(0);
	_spi->lock();
	_spi->write(_stateCache);
	_spi->unlock();
	_pinCS->write(1);
}

void DmLed::led_on(uint8_t led, uint32_t color) {
	char red = (color & LED_COLOR_RED) >> 16;
	char green = (color & LED_COLOR_GREEN) >> 8;
	char blue = (color & LED_COLOR_BLUE);
	
	switch(led) {
		case LED1:
			BIT_BOOL_SET(&_stateCache, LED1_RED_BIT, red==0);
			BIT_BOOL_SET(&_stateCache, LED1_GREEN_BIT, green==0);
			BIT_BOOL_SET(&_stateCache, LED1_BLUE_BIT, blue==0);
			break;
		
		case LED2:
			BIT_BOOL_SET(&_stateCache, LED2_RED_BIT, red==0);
			BIT_BOOL_SET(&_stateCache, LED2_GREEN_BIT, green==0);
			BIT_BOOL_SET(&_stateCache, LED2_BLUE_BIT, blue==0);
			break;
		
		default: 
			break;
	}
	apply();
}

void DmLed::led_off(uint8_t led) {
	led_on(led, LED_COLOR_BLACK);
}

void DmLed::led1_on(uint32_t color) {
	led_on(LED1, color);
}

void DmLed::led1_off(void) {
	led_off(LED1);
}

void DmLed::led2_on(uint32_t color) {
	led_on(LED2, color);
}

void DmLed::led2_off(void) {
	led_off(LED2);
}