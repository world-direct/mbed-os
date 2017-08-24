/*
 * flashconfig.h
 *
 * Created: 24.08.2017 20:20:02
 *  Author: Guenter.Prossliner
 */ 


#ifndef FLASHCONFIG_H_
#define FLASHCONFIG_H_

#include <stddef.h>

typedef enum {
	flashconfig_success,
	flashconfig_args_error,
	flashconfig_not_found,
	flashconfig_overrun
} flashconfig_result;

flashconfig_result flashconfig_get_value(const char * name, char * buffer, size_t buffer_size, size_t * value_size);

flashconfig_result flashconfig_set_value(const char * name, char * value);


#endif /* FLASHCONFIG_H_ */