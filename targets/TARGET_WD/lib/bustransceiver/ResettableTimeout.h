/* 
* ResettableTimeout.h
*
* Created: 31.07.2017 15:14:45
* Author: Patrick.Frischmann
*/


#ifndef __RESETTABLETIMEOUT_H__
#define __RESETTABLETIMEOUT_H__

#include "mbed.h"

class ResettableTimeout : public Timeout
{
//variables
public:
protected:
private:

//functions
public:
	ResettableTimeout(Callback<void()> func, timestamp_t us);
	~ResettableTimeout();
	//void start(void);
	//void stop(void);
	void reset(void);
	
protected:
private:
	ResettableTimeout( const ResettableTimeout &c );
	ResettableTimeout& operator=( const ResettableTimeout &c );

}; //ResettableTimeout

#endif //__RESETTABLETIMEOUT_H__
