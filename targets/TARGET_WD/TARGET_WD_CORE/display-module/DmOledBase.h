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
#define swap(a, b) { int16_t t = a; a = b; b = t; }

// Colors
#define BLACK		0
#define WHITE		1


/******************************************************
___________________DECLARATION_________________________
******************************************************/
class DmOledBase : public Stream {
public:
	DmOledBase(const uint16_t width, const uint16_t height) :
		_rawWidth(width), 
		_rawHeight(height),
		_width(width), 
		_height(height),
		rotation(0),
		cursorX(0),
		cursorY(0),
		_bgColor(BLACK),
		_fgColor(WHITE),
		textsize(1),
		wrap(true)
		{};
	virtual ~DmOledBase() {};
	
	virtual void init(void) = 0;
	
	inline uint16_t width(void) { return _width; }
	inline uint16_t height(void) { return _height; }
	void setWidth(uint16_t width) { _width = width; }
	void setHeight(uint16_t height) { _height = height; }
	
	void setTextColor(uint16_t background, uint16_t foreground) { _bgColor = background; _fgColor = foreground; }
	inline void setTextCursor(int16_t x, int16_t y) { cursorX = x; cursorY = y; };
	inline void setTextSize(uint8_t s) { textsize = (s > 0) ? s : 1; };
	
	virtual void setPixel(uint16_t x, uint16_t y, uint16_t color) = 0;
	virtual void invertDisplay(bool invert = true) {};	// optional
	
	// Stream implementation - provides printf() interface
	virtual int _putc(int value) { return writeChar(value); };
    virtual int _getc() { return -1; };
	
	virtual void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
	virtual void drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
	virtual void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
 
	virtual void drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
	virtual void fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
 
	virtual void fillScreen(uint16_t color);
	
	void drawChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg, uint8_t size);
	size_t writeChar(uint8_t c);
	
	void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* bitmap, uint16_t color);
 
    void setRotation(uint8_t r); // Set the display rotation, 1, 2, 3, or 4
    inline uint8_t getRotation(void) { rotation %= 4; return rotation; };
	
protected:
	uint16_t _rawWidth, _rawHeight;	// never changes
	uint16_t _width, _height;		// dependent on rotation
	
	int16_t  cursorX, cursorY;
    uint8_t  textsize;
	bool wrap; // If set, 'wrap' text at right edge of display
	
	uint8_t rotation;
	
	uint16_t _bgColor, _fgColor;
};

#endif // !DMOLEDBASE_H