/*
 * bootloader.h
 *
 * Created: 17.08.2017 17:25:55
 *  Author: Guenter.Prossliner
 */ 


#ifndef BLSRV_H_
#define BLSRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "WD_ABI.h"
#include "assert.h"

#define blsrv_erase_update_region		0x01
#define blsrv_write_update_region		0x02
#define blsrv_validate_update_image		0x03
#define blsrv_validate_boot_image		0x04
#define blsrv_apply_update				0x05
#define blsrv_write_config_data			0x06

struct blsrv_desc {

	// operation is a selector for the following fields
	int operation;
	union {
		
		struct {
			size_t offset;
			intptr_t src;
			size_t size;
		} write_update_region;

		struct {
			// output fields
			int validation_result;	// output field, will be set by the service
			intptr_t metadata_ptr;
			int command_word;	// the word following the image
		} validate_image;

		struct {
			int validation_result;	// output field, will be set by the service but only if != 0 , otherwise the call will not return but reset and apply!
		} apply_update;

		struct {
			int offset;	// offset from the start of the section
			intptr_t buffer;	// data to write
			size_t buffer_size;
		} write_config_data;
		
	} args;
};

static inline int blsrv_call(struct blsrv_desc * descriptor){
	int retcode;

	// validate the bootloader magic
	uint32_t bl_flags = *((uint32_t*)(WD_FLASH_BASE + WD_ABI_BL_HEADER_OFFSET + WD_ABI_BL_HEADER_FLDOFF_SRVCALL));
	assert(bl_flags == WD_ABI_BL_HEADER_MAGIC);

	typedef int (*ct)(void *);
	void ** vectable = (void**)(WD_FLASH_BASE + WD_ABI_BL_HEADER_OFFSET + WD_ABI_BL_HEADER_FLDOFF_SRVCALL);
	void * fnptr = *vectable;
	return ((ct)fnptr)(descriptor);
}

#ifdef __cplusplus
}
#endif


#endif /* BLSRV_H_ */