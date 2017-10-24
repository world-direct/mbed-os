
/*
 * bl_signature.S
 *
 * Created: 27.09.2017 15:02:06
 *  Author: Guenter.Prossliner
 */ 

#include "bl_asmoptions.inc"

#include "WD_ABI.h"

/*************************************************************************
	
	void * bl_signature_findkey (void * signature);
	
	Finds the key specified by signature.
	Returns a pointer to the signature-struct, which can be directly passed to the ecdsa_validate_image fn:

	r0:
		struct {
			uint32_t x[0x08];
			uint32_t y[0x08];
		} * key_vectors;

	r1:
		struct {
			uint32_t r[0x08];
			uint32_t s[0x08];
		} * signature_vectors;

	
	
*************************************************************************/
BL_FUNCTION(bl_signature_findkey):
push {r4, r5, lr}

	mov r4, r0	// r5: sig

	// compare signature version
	ldr r0, [r4]
	ldr r1, bl_signature_current_version
	cmp r0, r1
	bne 1f	// return NULL

	// locate the keystore
	ldr r0, bl_header_adr
	ldr r1, [r0, #WD_ABI_BL_HEADER_FLDOFF_KEYSTORE]	// r1: keystore address

	// compare keystore version
	ldr r0, bl_keystore_current_version
	ldr r2, [r1]
	cmp r0, r2
	bne 1f	// return NULL

	// compare the thumbprint
	add r1, #4		// r1: thumbprint in keystore
	add r0, r4, #4	// r0: thumbprint in signature
	mov r2, #0x20
	mov r4, r0		// r4 now contains the thumbprint in signature
	mov r5, r1		// r5 now contains the thumbprint in keystore
	bl bl_memcmp
	cmp r0, #0
	bne 1f	// return NULL

	// return ptrs
	add r1, r4, #0x20
	add r0, r5, #0x20
	b 0f
1:
	mov r0, 0
0:
pop {r4, r5, pc}


/*************************************************************************
	
	int bl_signature_validate (void * image, size_t size);
	
	Checks the image for a valid signature.
	Return-Values:
		0: validation ok!
		-1: nok
	
*************************************************************************/
BL_GLOBAL_FUNCTION(bl_signature_validate):
push {r4, r5, r6, lr}
sub sp, #32

	#define r_start r4
	#define r_len	r5
	#define r_pk	r6
	#define r_hash	sp

	mov r_start, r0	
	mov r_len, r1	
	mov r_pk, r2	

	// hash the image
	// r0 = start and r1 = len already
	mov r2, r_hash
	bl bl_sha256_digest	

	// find the key
	mov r0, r_start
	add r0, r_len
	bl bl_signature_findkey
	cmp r0, #0
	beq 1f	// return false

	// r0: key-vectors
	// r1: signature-vectors
	// int bl_ecdsa_validate_hash(hash, pk, sig);
	mov r2, r1
	mov r1, r0
	mov r0, r_hash
	bl bl_ecdsa_validate_hash

	b 0f
1:
	mov r0, #-1
0:
add sp, #32
pop {r4, r5, r6, pc}




.align 4
bl_header_adr:					.word WD_FLASH_BASE + WD_ABI_BL_HEADER_OFFSET
bl_signature_current_version:	.word WD_ABI_SIGNATURE_CURRENT_VERSION
bl_keystore_current_version:	.word WD_ABI_KEYSTORE_CURRENT_VERSION
