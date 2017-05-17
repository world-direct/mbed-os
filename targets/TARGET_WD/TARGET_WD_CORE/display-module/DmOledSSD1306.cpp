/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "DmOledSSD1306.h"
#include "BitIO.h"
#include "logo.h"

// the memory buffer for the display (initially includes the WD logo)
static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#if (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH > 96*16)
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0xF0, 0x98, 0x8C, 0x84, 0x84, 0x7C, 0x7C, 0x7C, 0x78, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x7C, 0xFC, 0xC0, 0xC0, 0xFC, 0x7C, 0xF8, 0x00, 0xF0, 0xFC, 0x1C, 0xC0, 0xF8, 0xFC, 0x1C,
0x0C, 0x0C, 0xFC, 0xF8, 0xE0, 0x00, 0xFC, 0xFC, 0xF8, 0x0C, 0x0C, 0x00, 0xFF, 0xFF, 0x00, 0x00,
0xE0, 0xF8, 0xFC, 0x0C, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x00,
0xF0, 0xF8, 0x1C, 0x0C, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0x0C, 0xFD, 0xFD, 0xFD, 0x00, 0x0C, 0xFC,
0xFC, 0x1C, 0x0C, 0x0C, 0xF0, 0xF8, 0xCC, 0xCC, 0xCC, 0xFC, 0xF8, 0x00, 0xE0, 0xF8, 0xFC, 0x0C,
0x0C, 0x1C, 0x1C, 0x08, 0x0C, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x98, 0xDC, 0xCC, 0xEC, 0xFC, 0xFC, 0xF8, 0x00, 0x0C, 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0x04, 0x00,
#if (SSD1306_LCDHEIGHT == 64)
0x00, 0x03, 0x07, 0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x0C, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0x0F, 0x0F, 0x01, 0x00, 0x0F, 0x0F, 0x0F, 0x01, 0x00, 0x01, 0x07, 0x0F, 0x0C,
0x0C, 0x0C, 0x0F, 0x07, 0x01, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0C, 0x0C,
0x03, 0x07, 0x0F, 0x0C, 0x0C, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x0F, 0x0E, 0x0C, 0x0C, 0x07, 0x0F, 0x0F, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x0F,
0x0F, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0C, 0x0C, 0x0E, 0x0E, 0x00, 0x03, 0x07, 0x0F, 0x0C,
0x0C, 0x0C, 0x0E, 0x04, 0x00, 0x07, 0x0F, 0x0C, 0x0C, 0x0E, 0x00, 0x0E, 0x0E, 0x0E, 0x04, 0x00,
0x07, 0x0F, 0x0C, 0x0C, 0x0E, 0x0F, 0x0F, 0x0C, 0x00, 0x07, 0x0F, 0x0F, 0x0C, 0x0C, 0x0E, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif
#endif
};


/******************************************************
___________________DEFINES_____________________________
******************************************************/


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/
DmOledSSD1306::DmOledSSD1306(PinName mosi, PinName sck, PinName cs, PinName dc, PinName rst) : DmOledBase(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
	_pinCS = new DigitalOut(cs, 1);
	_pinDC = new DigitalOut(dc, 0);
	_pinRST = new DigitalOut(rst, 1);	// initialize with output high
	_spi = new SPI(mosi, NC, sck);
	#ifdef SSD1306_SPI_USE_NSCK
	_spi->format(8, 3);					// mode 3 as we use nSCK!
	#else
	_spi->format(8, 1);
	#endif
	_spi->frequency(1000000);
}

DmOledSSD1306::~DmOledSSD1306() {
	delete _pinCS;
	delete _pinDC;
	delete _pinRST;
	delete _spi;
	
	_pinCS = NULL;
	_pinDC = NULL;
	_pinRST = NULL;
	_spi = NULL;
}

void DmOledSSD1306::select(void){
	_pinCS->write(0);
}

void DmOledSSD1306::deSelect(void){
	_pinCS->write(1);
}

void DmOledSSD1306::enterCommandMode(void) {
	_pinDC->write(0);
}

void DmOledSSD1306::enterDataMode(void) {
	_pinDC->write(1);
}

void DmOledSSD1306::writeBus(uint8_t data) {
	_spi->write(data);
}

void DmOledSSD1306::sendCommand(uint8_t index) {
	enterCommandMode();
	select();
	writeBus(index);
	deSelect();
}

void DmOledSSD1306::sendData(uint16_t data) {
	writeBus(data >> 8);
	writeBus(data);
}

void DmOledSSD1306::setPixel(uint16_t x, uint16_t y, uint16_t color) {
	
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
		return;

	 // check rotation, move pixel around if necessary
	switch (getRotation()) {
	case 1:
		swap(x, y);
		x = _rawWidth - x - 1;
		break;
	case 2:
		x = _rawWidth - x - 1;
		y = _rawHeight - y - 1;
		break;
	case 3:
		swap(x, y);
		y = _rawHeight - y - 1;
		break;
	}
	
	switch (color)
	{
		case WHITE:   buffer[x+ (y/8)*_rawWidth] |=  BIT_VALUE(y&7); break;
		case BLACK:   buffer[x+ (y/8)*_rawWidth] &= ~BIT_VALUE(y&7); break;
		case INVERSE: buffer[x+ (y/8)*_rawWidth] ^=  BIT_VALUE(y&7); break;
	}
	
}

void DmOledSSD1306::invertDisplay(bool invert) {
	sendCommand(invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void DmOledSSD1306::dim(bool dim) {
	// the range of contrast to too small to be really useful
	// it is useful to dim the display
	sendCommand(SSD1306_SETCONTRAST);
	sendCommand(dim ? 0 : 0xCF);
}

void DmOledSSD1306::refresh(void) {
	
	sendCommand(SSD1306_COLUMNADDR);
	sendCommand(0x00);
	sendCommand(SSD1306_LCDWIDTH - 1);
	
	sendCommand(SSD1306_PAGEADDR);
	sendCommand(0x00);
	#if SSD1306_LCDHEIGHT == 64
	sendCommand(0x07);
	#elif SSD1306_LCDHEIGHT == 32
	sendCommand(0x03);
	#elif SSD1306_LCDHEIGHT == 16
	sendCommand(0x01);
	#endif
	
	enterDataMode();
	select();
	for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i++) {
		writeBus(buffer[i]);
    }
	deSelect();
}

void DmOledSSD1306::clearDisplay(void) {
	memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}

void DmOledSSD1306::loadLogo(void) {
	memcpy(buffer, logo, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}

void DmOledSSD1306::init(void){
	
	setTextColor(BLACK, WHITE);
	
	// power-on sequence
	_pinRST->write(1);
	// VDD (3.3V) goes high upon start-up, lets just chill for a ms
	wait_ms(1);
	_pinRST->write(0);
	wait_ms(10);
	_pinRST->write(1);
	wait_ms(1);
	
	// init sequence
	sendCommand(SSD1306_DISPLAYOFF);
	
	// Display Clock Divide
	sendCommand(SSD1306_SETDISPLAYCLOCKDIV);
	sendCommand(0x80);								// the suggested ratio 0x80
	
	// Set MUX ratio
	sendCommand(SSD1306_SETMULTIPLEX);
	sendCommand(SSD1306_LCDHEIGHT - 1);
	
	// Set Memory Addressing Mode
	sendCommand(SSD1306_MEMORYMODE);
	sendCommand(0x00);								// Horizontal Addressing mode (auto page flip)
	
	// Set Column Address
	sendCommand(SSD1306_COLUMNADDR);
	sendCommand(0x00);
	sendCommand(SSD1306_LCDWIDTH - 1);
	
	// Set Page Address
	sendCommand(SSD1306_PAGEADDR);
	sendCommand(0x00);
	#if SSD1306_LCDHEIGHT == 64
	sendCommand(0x07);
	#elif SSD1306_LCDHEIGHT == 32
	sendCommand(0x03);
	#elif SSD1306_LCDHEIGHT == 16
	sendCommand(0x01);
	#endif
	
	// Set Display Offset
	sendCommand(SSD1306_SETDISPLAYOFFSET);
	sendCommand(0x0);								// no offset
	
	// Set Display Start Line
	sendCommand(SSD1306_SETSTARTLINE | 0x0);		// line #0
	
	// Set charge pump regulator
	sendCommand(SSD1306_CHARGEPUMP);
	sendCommand(0x14);								// enable charge pump
	
	// Set Segment re-map
	sendCommand(SSD1306_SEGREMAP | 0x0);//sendCommand(SSD1306_SEGREMAP | 0x1);			// column address 127 is mapped to SEG0
	
	// Set COM Output Scan Direction
	sendCommand(SSD1306_COMSCANINC);//sendCommand(SSD1306_COMSCANDEC);				// remapped mode. Scan from COM[N-1] to COM0
	
	// Set COM Pins hardware configuration and Contrast Control
	#if defined SSD1306_128_32
		sendCommand(SSD1306_SETCOMPINS);
		sendCommand(0x02);
		sendCommand(SSD1306_SETCONTRAST);
		sendCommand(0x8F);

	#elif defined SSD1306_128_64
		sendCommand(SSD1306_SETCOMPINS);
		sendCommand(0x12);							// Disable COM Left/Right remap, Alternative COM pin configuration
		sendCommand(SSD1306_SETCONTRAST);
		sendCommand(0xCF);

	#elif defined SSD1306_96_16
		sendCommand(SSD1306_SETCOMPINS);
		sendCommand(0x2);
		sendCommand(SSD1306_SETCONTRAST);
		sendCommand(0x10);

	#endif
	
	// Set Pre-charge Period 
	sendCommand(SSD1306_SETPRECHARGE);
	sendCommand(0x22);
	
	// Set VCOMH Deselect Level
	sendCommand(SSD1306_SETVCOMDESELECT);
	sendCommand(0x30);
	
	// Disable Entire Display On
	sendCommand(SSD1306_DISPLAYALLON_RESUME);
	
	// Set Normal Display
	sendCommand(SSD1306_NORMALDISPLAY);				// In normal display a RAM data of 1 indicates an 'ON' pixel
	
	// Deactivate scroll
	sendCommand(SSD1306_DEACTIVATE_SCROLL);
	
	// Display On
	sendCommand(SSD1306_DISPLAYON);
	
	wait_ms(100);									// SEG/COM on after 100ms
	
	// Show that we are ready
	refresh();										// refresh with initial buffer contents
}

void DmOledSSD1306::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
	bool bSwap = false;
	switch(rotation) {
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x
		bSwap = true;
		swap(x, y);
		x = width() - x - 1;
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for SSD1306_LCDHEIGHT.
		x = width() - x - 1;
		y = height() - y - 1;
		x -= (length-1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
		bSwap = true;
		swap(x, y);
		y = height() - y - 1;
		y -= (length-1);
		break;
	}

	if(bSwap) {
		drawVerticalLineInternal(x, y, length, color);
	} else {
		drawHorizontalLineInternal(x, y, length, color);
	}
}

void DmOledSSD1306::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
	bool bSwap = false;
	switch(rotation) {
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
		bSwap = true;
		swap(x, y);
		x = width() - x - 1;
		x -= (length-1);
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = width() - x - 1;
		y = height() - y - 1;
		y -= (length-1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y
		bSwap = true;
		swap(x, y);
		y = height() - y - 1;
		break;
	}

	if(bSwap) {
		drawHorizontalLineInternal(x, y, length, color);
	} else {
		drawVerticalLineInternal(x, y, length, color);
	}
}

void DmOledSSD1306::drawHorizontalLineInternal(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
	// Do bounds/limit checks
	if(y < 0 || y >= height()) { return; }

	// make sure we don't try to draw below 0
	if(x < 0) {
		length += x;
		x = 0;
	}

	// make sure we don't go off the edge of the display
	if( (x + length) > width()) {
		length = (width() - x);
	}

	// if our width is now negative, punt
	if(length <= 0) { return; }

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y/8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = BIT_VALUE(y&7);

	switch (color)
	{
		case WHITE:		while(length--) { *pBuf++ |= mask; }; break;
		case BLACK:		mask = ~mask;   while(length--) { *pBuf++ &= mask; }; break;
		case INVERSE:   while(length--) { *pBuf++ ^= mask; }; break;
	}
}

void DmOledSSD1306::drawVerticalLineInternal(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
	// do nothing if we're off the left or right side of the screen
	if(x < 0 || x >= width()) { return; }

	// make sure we don't try to draw below 0
	if(y < 0) {
		// __y is negative, this will subtract enough from __h to account for __y being 0
		length += y;
		y = 0;
	}

	// make sure we don't go past the height of the display
	if( (y + length) > height()) {
		length = (height() - y);
	}

	// if our height is now negative, punt
	if(length <= 0) {
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t __y = y;
	register uint8_t __h = length;


	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((__y/8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (__y&7);
	if(mod) {
		// mask off the high n bits we want to set
		mod = 8-mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if(__h < mod) {
			mask &= (0XFF >> (mod-__h));
		}

		switch (color)
		{
			case WHITE:   *pBuf |=  mask;  break;
			case BLACK:   *pBuf &= ~mask;  break;
			case INVERSE: *pBuf ^=  mask;  break;
		}

		// fast exit if we're done here!
		if(__h<mod) { return; }

		__h -= mod;

		pBuf += SSD1306_LCDWIDTH;
	}


	// write solid bytes while we can - effectively doing 8 rows at a time
	if(__h >= 8) {
		if (color == INVERSE)  {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
			do  {
				*pBuf=~(*pBuf);

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				__h -= 8;
				} while(__h >= 8);
		} else {
			// store a local value to work with
			register uint8_t val = (color == WHITE) ? 255 : 0;

			do  {
				// write our value in
				*pBuf = val;

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				__h -= 8;
			} while(__h >= 8);
		}
	}

	// now do the final partial byte, if necessary
	if(__h) {
		mod = __h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
		register uint8_t mask = postmask[mod];
		switch (color)
		{
			case WHITE:   *pBuf |=  mask;  break;
			case BLACK:   *pBuf &= ~mask;  break;
			case INVERSE: *pBuf ^=  mask;  break;
		}
	}
}


