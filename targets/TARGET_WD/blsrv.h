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


#define blsrv_erase_update_region		0x01
#define blsrv_write_update_region		0x02
#define blsrv_validate_update_image		0x03
#define blsrv_validate_boot_image		0x04
#define blsrv_get_update_metadata_ptr	0x05
#define blsrv_apply_update_with_reset	0x07


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
			int validation_result;	// output field, will be set by the service
			int update_status;	// output
		} validate_update_image;

		struct {
			int validation_result;	// output field, will be set by the service
		} validate_boot_image;

		struct {
			int validation_result;	// output field, will be set by the service but only if != 0 , otherwise the call will not return but reset and apply!
		} apply_update_with_reset;

		struct {
			intptr_t start;	// output field, will be set by the service
		} get_update_metadata_ptr;
		
	} args;
};

static inline int blsrv_call(struct blsrv_desc * descriptor){
	int retcode;

	typedef int (*ct)(void *);
	void ** vectable = (void**)0x08000200;
	void * fnptr = *vectable;
	return ((ct)fnptr)(descriptor);
}

#ifdef __cplusplus
}
#endif


#endif /* BLSRV_H_ */