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


enum blsrv_cmd_operation {
	blsrv_erase_update_region = 1,
	blsrv_flashmemcpy = 2,
	blsrv_validate_update_image = 3,
};

struct blsrv_desc {

	// operation is a selector for the following fields
	enum blsrv_cmd_operation operation;
	union {
		
		struct {
			intptr_t src;
			intptr_t dest;
			size_t size;
		} flashmemcpy;

		struct {
			int command_word;	// if != 0 this triggers updating the application after reset
		} validate_update_image;
		
	} args;
};

static inline int blsrv_call(struct blsrv_desc * descriptor){
	int retcode;

	asm (
		"mov r0, %0\n"	// load descriptor ptr
		"mov %1, #0x200\n"	// load fn address in any register
		"blx %1\n"
		"mov %1, r0"	// store res

		: "=r" (retcode)
		: "r" (descriptor)
	);
	
	return retcode;
}

#ifdef __cplusplus
}
#endif


#endif /* BLSRV_H_ */