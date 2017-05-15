/*
 * DmOledBase.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created:	11.05.2017
 * Author:	Patrick Frischmann
 * EMail:	patrick.frischmann@world-direct.at
 *
 * Description:
 *        Enables control of WD Display Module.
 *
 */ 

#ifndef DMOLEDBASE_H
#define DMOLEDBASE_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// Colors
//#define RED				0xf800
//#define GREEN			0x07e0
//#define BLUE			0x001f
//#define YELLOW			0xffe0
#define BLACK			0x0000
//#define WHITE			0xffff
//#define CYAN			0x07ff
//#define BRIGHT_RED		0xf810
//#define GRAY1			0x8410
//#define GRAY2			0x4208


/******************************************************
___________________DECLARATION_________________________
******************************************************/
class DmOledBase {
public:
	DmOledBase(const uint16_t width, const uint16_t height) : _width(width), _height(height) {};
	virtual ~DmOledBase() {};
	
	virtual void init(void) = 0;
	
	uint16_t width() { return _width; }
	uint16_t height() { return _height; }
	void setWidth(uint16_t width) { _width = width; }
	void setHeight(uint16_t height) { _height = height; }
	
	void setTextColor(uint16_t background, uint16_t foreground) { _bgColor = background; _fgColor = foreground; }
	
	virtual void select();
	virtual void deSelect();
	virtual void setPixel(uint16_t x, uint16_t y, uint16_t color );
	virtual void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) = 0;
	virtual void sendData(uint16_t data) = 0;
	
	void clearScreen(uint16_t color = BLACK);
	
	void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
	void drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
	void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
 
	void drawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
	void fillRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
 
	void drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
	void fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
 
	void drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
 
	void drawPoint(uint16_t x, uint16_t y, uint16_t radius=0);
	
	void drawChar(uint16_t x, uint16_t y, char ch, bool transparent);
	void drawNumber(uint16_t x, uint16_t y, int num, int digitsToShow, bool leadingZeros=false);
	void drawString(uint16_t x, uint16_t y, const char *p);
	void drawStringCentered(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char *p);
	
	void drawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* data);
 
protected:
	virtual void sendCommand(uint8_t index) = 0;
	DigitalOut* _pinCS;
	
private:
	uint16_t _width;
	uint16_t _height;
	
	uint16_t _bgColor;
	uint16_t _fgColor;
};

#endif // !DMOLEDBASE_H