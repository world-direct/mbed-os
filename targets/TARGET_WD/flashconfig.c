/*
 * flashconfig.c
 *
 * Created: 24.08.2017 20:24:13
 *  Author: Guenter.Prossliner
 */ 

#include "flashconfig.h"

#include <stdint.h>
#include <string.h>

#include "blsrv.h"

typedef union {
	uint32_t * w;
	char * c;
	const char * cc;
	intptr_t val;
} cfg_ptr;


// linker-script imports
extern int __flashconfig_start;
extern int __flashconfig_end;

// we need to use the address of the symbols, because the C compiler dereferences the extern vars automatically
#define start_intptr ((intptr_t)(&__flashconfig_start))
#define end_intptr ((intptr_t)(&__flashconfig_end))

static cfg_ptr m_endptr;

static void m_write_data(intptr_t address, cfg_ptr data, size_t size)
{
	struct blsrv_desc d;
	d.operation = blsrv_write_config_data;
	d.args.write_config_data.offset = address - start_intptr;
	d.args.write_config_data.buffer = data.val;
	d.args.write_config_data.buffer_size = size;

	blsrv_call(&d);
}

// moves the pointer backwards until we have a null-terminator
// and reads all of the padding \0s, so that the pointer is on the last char of the prev string
static int m_read_terminator(cfg_ptr * ptr)
{
	while(ptr->c--);
	while(!(ptr->c--));
}

flashconfig_result flashconfig_get_value(const char * name, const char ** value)
{
	if (!name) return flashconfig_args_error;
	if (!value) return flashconfig_args_error;
	
	size_t sname = strlen(name);
	if (!sname) return flashconfig_args_error;
		

	// m_endptr needs to be initialized once
	if(!m_endptr.val){
		m_endptr.val = end_intptr;
		while (m_endptr.val >= start_intptr && *(--m_endptr.w) == 0xFFFFFFFF);		
		m_endptr.w++;
	}

	if(m_endptr.val == start_intptr){
		return flashconfig_not_found;
	}

	enum {
		s_aftrval,	// after end of a value, incl. termination, so the next word is the endptr, or the first word of the prev. setting
		s_inval,	// we are in the value-string
		s_aftrkey,	// we are in the terminator of a key
		s_inkey,		// we are running the compare for the key
		s_nomatch,	// we have proved the current kvp not to match
		s_match		// we found a match
	} state = s_aftrval;

	const char * valueptr;
	const char * keyptr;

	cfg_ptr p = m_endptr;
	p.c--;

	for(; state != s_match && p.val > start_intptr;p.c--){
		char c = *p.c;

		// please note that the "missing" breaks here are intentional, and part of the state-machine!
		switch(state)	{
			case s_aftrval:
				if(!c) continue; // still in \0
				state = s_inval;	// we have anything other than \0

			case s_inval:
				if(c) continue; // still in key

				state = s_aftrkey;
				valueptr = p.cc+1;

			case s_aftrkey:
				if(!c) continue;	// we are still in \0

				state = s_inkey;		// we are in the key.
				keyptr = name + sname;	// initialize the pointer to compare the arg


			case s_inkey:
				if(!c)
					state = s_match;	// done compare
				else if(c != *--keyptr) 
					state = s_nomatch;	// no (longer) matching

				continue;

			case s_nomatch:
				if(!c) state = s_aftrval;
				continue;
		}

	}

	if(state == s_match || state == s_inkey){
		// found a match!
		*value = valueptr;
		return flashconfig_success;
	}


	return flashconfig_not_found;
}
