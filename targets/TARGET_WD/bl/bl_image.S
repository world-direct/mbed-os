/*
 * bl_image.S
 *
 * Created: 29.10.2017 21:08:03
 *  Author: Guenter.Prossliner


Implements the staged image loading and validation.
If a stage is completed with != 0, no further stages are executed.
*/

#include "bl_asmoptions.inc"

/*
Stages:
*/
.L_validation_stages_tbl:
	.align 4

	// checks if there is any valid header
	.word bl_read1_header_exisits

	// checks if there is a header postprocessed by the elf-tool (has valid size)
	.word bl_read2_header_validateable

	// checks if the CRC(image + CRC_padding) == 0
	.word bl_read3_crc_valid

	// checks if the masked CPUID matches the current system
	.word bl_read4_cpuid_valid

	// checks if the digital signature is valid
	.word bl_read5_dsa_valid

	// Mandatory end of table marker
	.word 0
/*

The primary return-value is the index of the last executed stage (stage_index)
Additional information about the image is returned by a image_state structure, passed as a pointer to bl_read_image.

	struct {

		// offset: 0x00
		// setby: bl_read_image entry method, updated by each successful stage
		// number of last check succeeded. Also returned in r0 by bl_read_image.
		int stage_index;

		// offset: 0x04
		// setby: bl_read_image entry method
		// start of image to read
		void * image_start;		

		// offset: 0x08
		// setby: bl_read1_header_exisits
		// content header-flags (lower start-word: contains image-kind, crc padding and dsa flags)
		int header_flags;

		// offset: 0x0C
		// setby: bl_read2_header_validateable
		// size of image, excluding CRC and signature
		int image_size;		
		
		// offset: 0x10
		// setby: bl_read2_header_validateable, updated by bl_read3_crc_valid and bl_read5_dsa_valid
		// pointer to word beyond image, including CRC and signature
		void * image_end;

	} image_state;


	Actions:

	Bootloader-Self-Check:
		1: RUN()		// development, bootloader not postprocessed
		4: CONTINUE()	// production, signature not required
		5: CONTINUE()	// production, signature required
		default: DIE()	// invalid bootloader header. Return to factory

	Application-Self-Check:
		1: RUN()		// dev


Returned:
	0: for an empty region of memory
	1: for an image not run though post-processing (elf-tool)
	3: incompatible CPUID check
	7: signature invalid, may apply if bootloader not self-signed
	F: everything valid, may apply update


 */ 



#include "WD_ABI.h"


/************************************************************************/
/*	int bl_read_image(void * image_start, struct readerstate * state)	*/
BL_GLOBAL_FUNCTION(bl_read_image):
PUSH {r4, r5, lr}

	MOV r4, r1	// r4: state

	// init state
	MOV r2, #0
	STR r2, [r4, #0x00]	// state->stage_index = 0
	STR r0, [r4, #0x04]	// state->image_start = image_start
	STR r2, [r4, #0x08]	// state->header_flags = 0
	STR r2, [r4, #0x0C]	// state->image_size = 0
	STR r2, [r4, #0x10]	// state->image_end = 0

	// call validation methods
	// r5 will hold the address of the method in the table
	// r1 will deref the address, and so contain the pointer to the function
	// r0 will hold the state arg passed to the function
	LDR r5, =.L_validation_stages_tbl

	1:	// next
		LDR r1, [r5], #4	// post-incr r5
		CMP r1, #0			// if(!r1)
		BEQ 0f				// return

		MOV r0, r4
		BLX r1				// call r1(state)
		CMP r0, #0			// if(!retvalue)
		BNE 0f				// return

		LDR r0, [r4]		// state->validation_result++
		ADD r0, #1
		STR r0, [r4, #0]

	B 1b	// goto next
	
0:	// done 
LDR r0, [r4]	// return stage_index in r0
POP {r4, r5, pc}


/************************************************************************/
BL_LOCAL_FUNCTION(bl_read1_header_exisits):
PUSH {lr}

	LDR r1, [r0, #4]				// r1: image-base
	ADD r1, #WD_ABI_HDR_OFFSET		// r1: &abi-header

	LDR r2, [r1]					// r2: hdr-magic
	STR r2, [r0, #8]				// store into state->header_flags

	// validate flags for WD_ABI_HDR_MAGIC
	LSR r1, r2, #16					// r1: high word of magic
	LDR r2, =#WD_ABI_HDR_MAGIC		// r2: magic expected
	SUB r0, r1, r2					// r0: 0 == MAGIC VALID, everything else: MAGIC INVALID

POP {pc}

/************************************************************************/
BL_LOCAL_FUNCTION(bl_read2_header_validateable):
PUSH {lr}

	LDR r1, [r0, #4]				// r1: image-base
	MOV r2, r1
	ADD r2, #WD_ABI_HDR_OFFSET		// r2: &abi-header

	LDR r2, [r2, #4]				// r2: image-size
	CMP r2, #0xFFFFFFFF
	BEQ	1f

	STR r2, [r0, #0x0C]				// state->image_size = r2
	ADD r2, r1						// r2: image_end
	STR r2, [r0, #0x10]				// state->image_end = state->image_size + state->image_base

	B 0f							// return true
1: MOV r0, 1; B 2f
0: MOV r0, 0
2: POP {pc}


/************************************************************************/
BL_LOCAL_FUNCTION(bl_read3_crc_valid):
PUSH {r4, r5, r6, lr}

	MOV r4, r0						// r4: state

	// check header_flags if we have crc padding
	LDR r1, [r0, #0x08]				// r1: header_flags
	TST r1, WD_ABI_HDR_FLAG_CRC
	BEQ 0f							// return 0

	// perform crc validation
	LDR r5, [r4, #0x04]				// ptr, post-increment
	LDR r6, [r4, #0x0C]				// size, post-decrement
	ADD r6, #4						// add to include CRC padding

	BL bl_hal_crc_init

	3: // while(size) {

		LDR r0, [r5], #4	// r0 = *r5++4

		// LDR reads not the pysical order, it read LittleEndian, so we get 0x01020304, but in the file we have 0x4030201! 
		// to allow that we can form the CRC on the file (it seems better IMO), to REV to file-order here
		REV r0, r0	
		BL bl_hal_crc_update	

		SUBS r6, #4
	BNE 3b // while(size)

	// state->image_end+=4
	LDR r1, [r4, #0x10]
	ADD r1, #0x04
	STR r1, [r4, #0x10]

	// r0 contains the crc from the last call to bl_hal_crc_update, which should be 0!
	// we can return it directly
	B 2f

1: MOV r0, 1; B 2f
0: MOV r0, 0
2: POP {r4, r5, r6, pc}


/************************************************************************/
BL_LOCAL_FUNCTION(bl_read4_cpuid_valid):
PUSH {lr}
								// r0: state
	LDR r0, [r0, #0x04]			// r0: image_start
	ADD r0, WD_ABI_HDR_OFFSET	// r0: image_header
	LDR r1, [r0, #0x08]			// r1: CPUID-mask
	LDR r2, [r0, #0x0C]			// r2: CPUID-comparant

	// Load CPUID into r0
	LDR r0, bl_data_cpuid_address						// truth table example
	LDR r0, [r0]										// r0: running cpuid		11110000	r0: running
														// r2: cpuid compare		10101010	r2: compare
														// r1: cpuid-mask			11001100	r1: mask
	EOR r0, r2											//							01011010	r0 = r0 xor r2 (all 1 bits are different)
	AND r0, r1											//							01001000	r0 = r0 and r1 (two bits dont match, so this would be incompatible)

POP {pc}

/************************************************************************/
BL_LOCAL_FUNCTION(bl_read5_dsa_valid):
PUSH {lr}

	LDR r1, [r0, #0x10]			// r1: image_end
	LDR r2, [r0, #0x04]			// r2: image_start
	SUB r1, r2					// r1: size
	MOV r0, r2					// r0: image_start

	BL bl_signature_validate	//(void * image, size_t size);
	// return-value from bl_signature_validate is passed in r0

	// state->image_end+=0x44
	LDR r1, [r4, #0x0C]
	ADD r1, #0x44
	STR r1, [r4, #0x0C]


POP {pc}

.align 4
bl_data_cpuid_address: .word 0xE000ED00
