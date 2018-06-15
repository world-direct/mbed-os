#ifndef ACTOR_DIGITAL_OUT_H
#define ACTOR_DIGITAL_OUT_H


class ActorDigitalOut {
	public:

		virtual ~ActorDigitalOut(){};
	
		virtual int getValue(void) = 0;
	
		virtual void setValue(int value) = 0;	
};

#endif