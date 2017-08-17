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
	blsrv_flashmemcpy = 2
};

struct blsrv_desc {

	enum blsrv_cmd_operation operation;
	union {
		
		// operation is a selector for the following fields

		struct {
			void * src;
			void * dest;
			size_t size;
		} flashmemcpy;

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