#ifndef SERIAL_IO_H
#define SERIAL_IO_H

#include <inttypes.h>

typedef struct serial_io_fns
{
	int(*write)(uint8_t* buf, size_t length, uint32_t timeout);
	int(*read)(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout);
} serial_io_fns_t;

#endif //SERIAL_IO_H