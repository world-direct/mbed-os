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

static void m_checkinit()
{
	// m_endptr needs to be initialized once
	if(!m_endptr.val){
		m_endptr.val = end_intptr;
		while (m_endptr.val >= start_intptr && *(--m_endptr.w) == 0xFFFFFFFF);
		m_endptr.w++;
	}
}

flashconfig_result flashconfig_get_value(const char * name, const char ** value)
{
	if (!name) return flashconfig_args_error;
	if (!value) return flashconfig_args_error;
	
	size_t sname = strlen(name);
	if (!sname) return flashconfig_args_error;
		
	m_checkinit();

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

flashconfig_result flashconfig_set_value(const char * name, char * value)
{
	// let's start with some validation
	if(!name) return flashconfig_args_error;
	if(!value) return flashconfig_args_error;

	size_t sname = strlen(name);
	size_t svalue = strlen(value);

	if(!sname || sname >= FLASHCONFIG_NAME_MAX) return flashconfig_args_error;
	if(!svalue|| sname >= FLASHCONFIG_VALUE_MAX) return flashconfig_args_error;

	m_checkinit();

	// validate that we have enough storage left
	cfg_ptr p = m_endptr;	// m_endptr is always word-aligned and on FFFFFFFF
	#define align_p() do{if(p.val %4) p.val += (4-p.val % 4);} while(0)

	p.c += sname + 1;	// name with \0
	align_p();
	size_t valoffset = p.val - m_endptr.val;
	p.c += svalue + 1;	// value with \0
	align_p();

	if(p.val >= end_intptr)
		return flashconfig_overrun;

	size_t memsize = p.val - m_endptr.val;
	uint32_t buffer[(FLASHCONFIG_NAME_MAX + FLASHCONFIG_VALUE_MAX + 8) / 4];	// alloc as i32 for alignment
	char * bufferp = (char*)buffer;

	memset(bufferp, 0, memsize);
	p.c = bufferp;

	memcpy(bufferp, name, sname);
	memcpy(bufferp + valoffset, value, svalue);

	// and write to flash
	struct blsrv_desc d;
	d.operation = blsrv_write_config_data;
	d.args.write_config_data.offset = m_endptr.val - start_intptr;
	d.args.write_config_data.buffer = (intptr_t)bufferp;
	d.args.write_config_data.buffer_size = memsize;
	blsrv_call(&d);

	m_endptr.c += memsize;
}