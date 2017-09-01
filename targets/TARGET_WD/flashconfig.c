/*
 * flashconfig.c
 *
 * Created: 24.08.2017 20:24:13
 *  Author: Guenter.Prossliner
 */ 

#include "flashconfig.h"

#include <stdint.h>
#include <string.h>

// linker-script imports
extern intptr_t __flashconfig_start;
extern intptr_t __flashconfig_end;

static char * m_endptr;

static void m_write_char(intptr_t address, char c)
{

}

static flashconfig_result m_flashconfig_get_value(const char * name, char * buffer, size_t buffer_size, size_t * value_size)
{

	if(!name || !strlen(name))
		return flashconfig_args_error;

	// m_endptr needs to be initialized once
	if(!m_endptr){
		for(char * c = (char*)__flashconfig_start; c<(char*)__flashconfig_end; c++){
			if(*c == 0xFF){
				m_endptr = c;
				break;
			}
		}
	}

	if(m_endptr == __flashconfig_start){
		return flashconfig_not_found;
	}

	size_t sname=strlen(name);
	int state = 0;
	size_t valuelen = 0;
	size_t nameindex;
	char * valueptr = NULL;

	for(char * ptr=m_endptr-1;m_endptr >= __flashconfig_start; m_endptr--){
		char c = *ptr;

		switch(state){
			case 0: // state 0 (end of setting): we count the len of the value
				if(c != '='){
					valuelen++;
				} else {
					valueptr = ptr + 1;
					state = 1;
					nameindex = sname-1;
				}
				continue;

			case 1: // state 1 (end of name): we compare the found char in reverse order to the lookup
				// check if we (still) match
				if(c == name[nameindex]){
					// we do!
					if(nameindex-- == 0){
						// match complete
						state = -1;
						break;
					}
				} else {
					state = 2;
				}
				continue;

			case 2: // state 2 (skip): we skip the remaining chars of the setting that was not requested
				if(c == '\n'){
					state = 0;
					valuelen = 0;
				}
				continue;
				
		}
	}

	if(state == -1){
		// got a match!
		*value_size = valuelen + 1;	// null-terminator
		if(*value_size > buffer_size){
			return flashconfig_overrun;
		}

		if(buffer){
			memcpy(buffer, valueptr, valuelen);
			buffer[valuelen] = '\0';	// null-terminator
		}

		return flashconfig_success;
	}
		
	return flashconfig_not_found;
}

flashconfig_result flashconfig_get_value(const char * name, char * buffer, size_t buffer_size, size_t * value_size)
{
	return m_flashconfig_get_value(name, buffer, buffer_size, value_size);
}

flashconfig_result flashconfig_set_value(const char * name, char * value)
{
	
}