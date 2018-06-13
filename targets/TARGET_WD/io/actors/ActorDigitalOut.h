#ifndef ACTOR_DIGITAL_OUT_H
#define ACTOR_DIGITAL_OUT_H

/******************************************************
___________________DECLARATION_________________________
******************************************************/
class ActorDigitalOut {
public:
	
	virtual ~ActorDigitalOut(){};
	
	virtual int getValue(void) { };
	virtual void setValue(int value);	
};

#endif