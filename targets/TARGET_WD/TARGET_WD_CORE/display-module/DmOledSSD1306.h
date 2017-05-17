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
 *        Library for Monochrome OLEDs based on SSD1306 driver.
 *
 */ 
#ifndef DMOLEDSSL1306_H
#define DMOLEDSSL1306_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "DmOledBase.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/
#define SSD1306_128_64
//   #define SSD1306_128_32
//   #define SSD1306_96_16

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 32
#endif
#if defined SSD1306_96_16
  #define SSD1306_LCDWIDTH                  96
  #define SSD1306_LCDHEIGHT                 16
#endif

//#define SSD1306_SPI_USE_NSCK			// uncomment if negated SCK signal is used

// Colors
#define BLACK		0
#define WHITE		1
#define INVERSE		2

// Commands
#define SSD1306_SETCONTRAST								0x81
#define SSD1306_DISPLAYALLON_RESUME						0xA4
#define SSD1306_DISPLAYALLON							0xA5
#define SSD1306_NORMALDISPLAY							0xA6
#define SSD1306_INVERTDISPLAY							0xA7
#define SSD1306_DISPLAYOFF								0xAE
#define SSD1306_DISPLAYON								0xAF

#define SSD1306_SETDISPLAYOFFSET						0xD3
#define SSD1306_SETCOMPINS								0xDA

#define SSD1306_SETVCOMDESELECT							0xDB

#define SSD1306_SETDISPLAYCLOCKDIV						0xD5
#define SSD1306_SETPRECHARGE							0xD9

#define SSD1306_SETMULTIPLEX							0xA8

#define SSD1306_SETLOWCOLUMN							0x00
#define SSD1306_SETHIGHCOLUMN							0x10

#define SSD1306_SETSTARTLINE							0x40

#define SSD1306_MEMORYMODE								0x20
#define SSD1306_COLUMNADDR								0x21
#define SSD1306_PAGEADDR								0x22

#define SSD1306_COMSCANINC								0xC0
#define SSD1306_COMSCANDEC								0xC8

#define SSD1306_SEGREMAP								0xA0

#define SSD1306_CHARGEPUMP								0x8D

#define SSD1306_EXTERNALVCC								0x1
#define SSD1306_SWITCHCAPVCC							0x2

// Scrolling commands
#define SSD1306_ACTIVATE_SCROLL							0x2F
#define SSD1306_DEACTIVATE_SCROLL						0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA				0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL					0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL					0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL	0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL		0x2A


/******************************************************
___________________DECLARATION_________________________
******************************************************/
class DmOledSSD1306 : public DmOledBase {
public:
	DmOledSSD1306(PinName mosi, PinName sck, PinName cs, PinName dc, PinName rst);
	virtual ~DmOledSSD1306();
	virtual void init(void);
	
	void refresh(void);
	void clearDisplay(void);
	void loadLogo(void);
	
	void invertDisplay(bool invert = true);
	void dim(bool dim = true);
	
	void setPixel(uint16_t x, uint16_t y, uint16_t color);
	
	virtual void drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
	virtual void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
	
private:
	void select(void);
	void deSelect(void);
	void enterCommandMode(void);
	void enterDataMode(void);
	
	void writeBus(uint8_t data);
	void sendCommand(uint8_t index);
	void sendData(uint16_t data);
	
	inline void drawVerticalLineInternal(uint16_t x, uint16_t y, uint16_t length, uint16_t color) __attribute__((always_inline));
	inline void drawHorizontalLineInternal(uint16_t x, uint16_t y, uint16_t length, uint16_t color) __attribute__((always_inline));
	
	DigitalOut* _pinCS;
	DigitalOut* _pinDC;
	DigitalOut* _pinRST;
	SPI* _spi;
};

#endif // !DMOLEDSSL1306_H