/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "DmButton.h"
#include "BitIO.h"


/******************************************************
___________________DEFINES_____________________________
******************************************************/
#define BUTTON1_BIT		0x0
#define BUTTON2_BIT		0x1
#define BUTTON3_BIT		0x2
#define BUTTON4_BIT		0x3
#define LIGHTSENSOR_MASK	0x30


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/

static void donothing() {}

DmButton::DmButton(PinName miso, PinName sck, PinName cs, PinName irq) : _pinCS(cs, 1), _pinIRQ(irq), _spi(NC, miso, sck)  {
	
//	_spi.format(8, 2);
//	_spi.frequency(115200);
	
	_pinIRQ.fall(this, &DmButton::onButtonPressed);
	_pinIRQ.enable_irq();
	
	for (int i = 0; i < sizeof _irq / sizeof _irq[0]; i++) {
        _irq[i].attach(donothing);
    }
}

DmButton::~DmButton() {}


void DmButton::onButtonPressed(void) {
	
	// read register state
	_pinCS.write(0);
	_spi.lock();
	
	int result = _spi.write(0xFF);
	
	_spi.unlock();
	_pinCS.write(1);
	
	if (result < 0) return; // read failed
	
	// call attached function
	uint8_t res = ~((uint8_t) result); // active low
	
	if (BIT_CHECK(&res, BUTTON1_BIT)) {
		_irq[Button1].call();
	} else if (BIT_CHECK(&res, BUTTON2_BIT)) {
		_irq[Button2].call();
	} else if (BIT_CHECK(&res, BUTTON3_BIT)) {
		_irq[Button3].call();
	} if (BIT_CHECK(&res, BUTTON4_BIT)) {
		_irq[Button4].call();
	}
}

void DmButton::attach(Callback<void()> func, IrqSource type /* = Button1 */) {
	if(func){
		_irq[type].attach(func);
	} else {
		_irq[type].attach(donothing);
	}
}
