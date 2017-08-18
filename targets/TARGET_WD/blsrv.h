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


#define blsrv_erase_update_region	0x01
#define blsrv_write_update_region	0x02
#define blsrv_validate_update_image	0x03

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
			int command_word;	// if != 0 this triggers updating the application after reset
		} validate_update_image;
		
	} args;
};

static inline int blsrv_call(struct blsrv_desc * descriptor){
	int retcode;

	typedef int (*ct)(void *);
	void ** vectable = (void**)0x08000200;
	void * fnptr = *vectable;
	return ((ct)fnptr)(descriptor);

	//asm (
		//"mov r0, %0\n"	// load descriptor ptr
		//"mov %1, #0x200\n"	// load table address in any register
		//"ldr %1, [%1]\n"	// load fn address in register
		//"blx %1\n"		// and call
		//"mov %1, r0"	// store res
//
		//: "=r" (retcode)
		//: "r" (descriptor)
	//);
	
	return retcode;
}

#ifdef __cplusplus
}
#endif


#endif /* BLSRV_H_ */