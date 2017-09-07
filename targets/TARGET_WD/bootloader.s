
/*
 * bootloader.s
 *
 * Created: 10.08.2017 11:40:20
 *  Author: Guenter.Prossliner

# Implementation note:

The bootloader is not in a part of the application that is performance-critical.
For sure we should not spend time in doing unnesserary things, but we will focus on maintainability instead of reducing some CPU cycles

# Organization:

The bootloader is seperated into two files:

## bootloader.s (this file) 
Contains the entry-point and implements the bootloader workflow.
The ISR-Vectors go to section .bl_vectors, all other objects to section .bl.
All functions are prefixed with bl_

## bootloader_BOARD.s (like bootloader_cb.s)
Implements function prototypes called by bootloader.s to access the hardware.
There will be more than one bootloader_BARDS.s file in the project, but only one should be linked.
All objects need to be placed in the .bl section.
All functions are prefixed with bl_hal_

# Calling convention

We will use the standard ARM calling convention for the implementation:
- Arguments will be passed from R0 to R3
- Return value will be passed in R0
- Registers R4-R11 must be perserved by the callee (PUSHM {r4-r11, lr} .... POPM {r4-r11, pc})

# Document conventions

To document the signatures of the bl_hal_ functions we will write a pseudo C signature as a comment in the sourcefile.
The use of constants (like HW-Registers or there values need to be documented in a way that allows them to be found in the datasheets.

# Update state

The process of applying an update needs to maintain some state, like if we can an valid update, the bootloader should apply an update,
or the result of this process.

First we wanted to use Option-Bits for this, but there seem be no user-defined option bits available.
We don't want to use RAM, because of it's volatile nature, and also no external memory.

The current concept don't explicitly stores this state in a single memory location.
It is reconstructed every time the bootloader runs. Changing this state is done by earse or write operations to 
one of the memory banks.

1. Valid Metadata in Boot-Image? =no=> unkown, try reapply
2. Boot bank valid (see validation procedure)? =no=> app invalid, try reapply

=> app is valid an can be launched, continue to read update status

3. Valid Metadata in Update-Image? =no=> unkown update status (factory default)
4. Is Update == Boot =no=> update already done
4. Update bank valid (see validation procedure)? =no=> update invalid, append error-code to Update bank
5. Read Update command word from flash
6. If == APPLY apply update, write Update Status word, and restart

## Metadata Validation

1. Check Metadata signurature word. Must be 0x01020304, or MD_NOTFOUND
2. Read Image size. Must be < sizeof(imageregion), or INVALID_SIZE

## Bank Validation

Depends on successfull metadata validation.
Runs a CRC (we will use the STM32 hardware CRC driver, which uses the CRC_MPEG2 implementation, must change Build-tool).
If final CRC == 0 => IMAGE_VALUE
if CRC word == 0 => IMAGE_UNVERIFYABLE (no crc signature found)
=> IMAGE_INVALID

- Default: Unkown
	- Factory default. Bootloader starts app directly
	- Update area erased

- UpdateRequested
	- enabled by user request (sw_apply_update)

- Default: UpdateSuccess
	- Update has been applied
	- Update area not erased

- UpdateValidationError
	- Update did not pass validation
	- Retry will not be successfull

- UpdateProgrammingError
	- The Flash hardware controller reported an error while programming
	- additional state may be stored in RAM

- UpdateSystemError
	- An unexpected error or interrupt occured on running the update
	- bootloader may be corrupt, or there is an programming bug
	- retry most likely to fail, but you may try ;-)

 */ 


#include "WD_ABI.h"

.syntax unified
.thumb

.section  .bl_vectors,"a",%progbits
.type  g_bl_vectors, %object

g_bl_vectors:

	.word 0x20001000		// SP for bootloader
	.word bl_start	// reset vector
	.word bl_Other_Handler
	.word bl_HardFault_Handler
	.word bl_Other_Handler
	.word bl_Other_Handler
	.word bl_Other_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word bl_Other_Handler
	.word bl_Other_Handler
	.word 0
	.word bl_Other_Handler
	.word bl_Other_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word bl_flash_handler

.size  g_bl_vectors, .-g_bl_vectors

/*************************************************************************
	WDABI data elements which need to be an fixed offset (0x200)
*/

	// aligns to 0x200
	.p2align 9
	.word bl_srv_call			// WDABI: entry vector for indirect calls from blsrv
	.word __bootloader_size		// WDABI: size of bootloader for image building
	.word __flashconfig_start	// WDABI: start of bank2 for image building
	
.section .bl_text,"ax",%progbits

/*************************************************************************
	int bl_srv_call(void * cmd)
	this is the entrypoint service calls from the application to the bootloader
	it's it called indirectly from the application by the address emitted to 0x200.

	The cmd argument pointer points to a variable-sized structure to descibe the args,
	where the first word is a selector for the operation, which may need following fields

#define blsrv_erase_update_region		0x01
#define blsrv_write_update_region		0x02
#define blsrv_validate_update_image		0x03
#define blsrv_validate_boot_image		0x04
#define blsrv_apply_update_with_reset	0x05
#define blsrv_get_update_status			0x06
#define blsrv_write_config_data			0x06

	(see struct blsrv_desc in blsrv.h)
*/
.type bl_srv_call, %function
bl_srv_call:
PUSH {r4, r5, r6, lr}

	MOV r4, r0			// store desc in r4
	LDR r1, [r4], #4	// and load operation

	// r4: contains the ptr to be first arg in the call-descriptor
	// r5 and r6 can be used to store vars in the case blocks (.L_blsrv_*)

	// test for blsrv_erase_update_region = 1
	MOV r5, #1
	CMP r1, r5
	BEQ .L_blsrv_erase_update_region

	// test for blsrv_write_update_region = 2
	MOV r5, #2
	CMP r1, r5
	BEQ .L_blsrv_write_update_region

	// test for blsrv_validate_update_image = 3
	MOV r5, #3
	CMP r1, r5
	BEQ .L_blsrv_validate_update_image

	// test for blsrv_validate_boot_image	0x04
	MOV r5, #4
	CMP r1, r5
	BEQ .L_blsrv_validate_boot_image

	// test for blsrv_apply_update	0x05
	MOV r5, #5
	CMP r1, r5
	BEQ .L_blsrv_apply_update

	// test for blsrv_write_config_data			0x06
	MOV r5, #6
	CMP r1, r5
	BEQ .L_blsrv_write_config_data

	// return false (invalid operation)
	MOV r0, 0
	B 0f

	.L_blsrv_erase_update_region:
		BL bl_hal_erase_update_image
		B 1f

	.L_blsrv_write_update_region:
		// load args
		LDR r0, [r4], #4	// load offset
		LDR r1, bl_data_update_image_start	// load base address
		ADD r0, r1			// dest = offset + base address
		LDR r1, [r4], #4	// src
		LDR r2, [r4], #4	// size
		BL bl_hal_flash_memcpy
		B 1f

	.L_blsrv_validate_update_image:
		LDR r5, bl_data_update_image_start
		B .L_validate_image

	.L_blsrv_validate_boot_image:
		LDR r5, bl_data_image_start
		B .L_validate_image


	.L_validate_image:
		// r5 contains image-base
		MOV r0, r5

		BL bl_validate_image // r0=ret_code, r1=size
		MOV r6, r1	// store size

		// output fields:
		STR r0, [r4], #4	// validation_result

		MOV r1, r5
		LDR r2, bl_data_metadata_offset
		ADD r1, r2
		STR r1, [r4], #4		// metdata_ptr

		MOV r1, r5	// r1=base
		MOV r2, r6	// r2=size
		ADD r1, r6	// r1=&command_word (src)
		LDR r1, [r1] // command_word
		STR r1, [r4], #4	// command_word
			
		B 1f

	.L_blsrv_apply_update:

		LDR r0, bl_data_update_image_start
		BL bl_validate_image
		MOV r6, r1	// size
		MOVS r5, r0	// result

		// set validation result to output struct field
		STR r5, [r4], #4	

		BNE 1f	// if (result!=0) return

		MOV r0, r6
		ADR r1, bl_data_commandword_apply
		BL bl_set_command_word

		B 1f

	.L_blsrv_write_config_data:
		// load args
		LDR r0, [r4], #4	// load offset
		LDR r1, bl_data_flashconfig_start	// load base address
		ADD r0, r1			// dest = offset + base address
		LDR r1, [r4], #4	// src
		LDR r2, [r4], #4	// size
		BL bl_hal_flash_memcpy
		B 1f

1:
MOV r0, 1
0:
POP {r4, r5, r6, pc}


/*************************************************************************
	void bl_start(void)
	this is the entrypoint of the bootloader
*/
.global bl_start
.type bl_start, %function
bl_start:

	// variable registers
	// r4: bool image verification result
	// r5: update image verification result
	// r6: end of update image pointer (to read command word or to write update status)

	/////////////////////////////////////////////////////
	// perform hw initialization
	BL bl_hal_init

	// turn on BUS_LED to signal start of bootloader
	// MED will turn it of on initialization (somewhere in software_init_hook), so you will just see it flashing!
	/////////////////////////////////////////////////////
	BL bl_hal_ui

	// get the system state
	/////////////////////////////////////////////////////

	// start validating boot image
	LDR r0, bl_data_image_start
	BL bl_validate_image
	MOV r4, r0

	// validate update image
	LDR r0, bl_data_update_image_start
	BL bl_validate_image
	MOV r6, r1	// size
	MOVS r5, r0	// result, set's Z bit if == 0
	
	BNE .L_start_app	// if (result!=0) goto .L_start_app

	// evaluate command-word
	LDR r0, bl_data_update_image_start
	ADD r0, r6		// r0: &command_word
	LDR r0, [r0]	// r0: command_word
	MOV r1, #0x000000FF
	CMP r1, r0
	BNE .L_start_app	// skip update, because if command-word

	MOV r0, r6	//load size
	BL bl_update	// bl_update(size)

	// update command-word to zero
	MOV r0, r6
	ADR r1, bl_data_commandword_success	
	BL bl_set_command_word

	.L_start_app:

	// start application
	/////////////////////////////////////////////
	LDR r0, bl_data_image_start

	// load SP
	LDR r1, [r0, #0]
	MOV sp, r1

	// load PC
	LDR r1, [r0, #4]
	MOV pc, r1



/*************************************************************************
	void bl_set_command_word(int image-size, int * command_word)
	updates the command-word in flash.
	
	NOTE: There is no erase here, so after erase (after factory programming or sw-update), you can only update bits from 1 to 0.
	the command-word is: 
		- 0xFFFFFFFF on erase, no further action
		- 0x0000FFFF if the bootloader should  load the new image, set by bootloader after validation
		- 0x00000000 if bootloader applied the image, can only be reset by a new image
*/
.type bl_set_command_word, %function
bl_set_command_word:
PUSH {r4, lr}

	LDR r2, bl_data_update_image_start
	ADD r0, r2		// r0 (dest): &command_word_in_flash
	
	MOV r4, r1		// command_word

	// local buffer (we uses stack) needed for flash_memcpy
	MOV r1, r4
	MOV r2, #4		// r2: (size): always 4
	BL bl_hal_flash_memcpy	// perform the write

POP {r4, pc}

/*************************************************************************
	void bl_update(int size)

	this is the entypoint for erase and write of the image
	size is passed form the validation procedure
*/
.global bl_update
.type bl_update, %function
bl_update:
PUSH {r4, lr}

	// variable regisers
	// r4: size
	MOV r4, r0

	// erase app bank
	BL bl_hal_erase_boot_image

	// flash progamming
	LDR r0, bl_data_image_start
	LDR r1, bl_data_update_image_start
	MOV r2, r4
	BL bl_hal_flash_memcpy
	
0:
POP {r4, pc}


/*************************************************************************
	struct {
		int status; // returned in r0
		int size; // returned in r1
	} bl_validate_image(void*)

	The size is only valid if status == 0

	This method performs the image validation for the given start-address (which is bl_data_image_start or bl_data_update_image_start)
	It returns one of the following enumation values

	0: Valid
	1: NoMetadata (Signature != 0x01020304)
	2: InvalidMetadata (Length > max)
	3: InvalidImage (CRC validation failed)
	4: UnverifiableImage (CRC validation failed && CRC==0xFFFFFFFF)
*/
.type bl_validate_image, %function
bl_validate_image:
PUSH {r4, r5, r6, r7, r8, lr}

	////////////////////////////
	// local constant vars ;-)
	MOV r4, r0	// r4 will hold the base-address
				// r8 will be initialized later for the image-size

	////////////////////////////
	// metadata validation

	// start validating metadata by loading the signature word into r5
	MOV r5, r4
	LDR r6, bl_data_metadata_offset
	ADD r5, r6
	LDR r5, [r5]

	// load comparant to r6
	LDR r6, bl_data_metadata_magic

	// and do the compare
	CMP r5, r6
	BEQ .L_validate_metadata	// continue
		
		// return error 1 (NoMetadata)
		MOV r0, 1
		B .L_ret

.L_validate_metadata:

	// load length
	MOV r5, r4
	LDR r6, bl_data_metadata_offset
	ADD r5, r6
	LDR r8, [r5, #4]	// len has offset of 4, see metadata.s
	MOV r7, r8 // r7 will be used in .L_validate_image, but decremented. r8 will not be overwritten and can be returned

	// load comparant to r6
	LDR r6, bl_data_application_max_size

	// and do the compare
	CMP r6, r7
	BHI .L_validate_image_data	// continue
		// return error 2 (InvalidMetadata)
		MOV r0, 2
		B .L_ret

.L_validate_image_data:

	// r5 current address, will be incremented by 4 in the loop
	MOV r5, r4

	// r6 will contain the current crc

	// r7 remaining length, will be post-decremented (by 4) in the loop until 0 is reached
	// r7 is already initialized from .L_validate_metadata

	BL bl_hal_crc_init

	// load current word
.L_validate_next_word:
	LDR r0, [r5], #4	// this is post-index, so it should to r5+=4

	// LDR reads not the pysical order, it read LittleEndian, so we get 0x01020304, but in the file we have 0x4030201! 
	// to allow that we can form the CRC on the file (it seems better IMO), to REV to file-order here
	REV r0, r0	

	BL bl_hal_crc_update	
	MOV r6, r0	// store crc in r6

	// check if we are done. SUB updates the flags, so no need to compare here
	SUBS r7, #4
	BNE .L_validate_next_word

	// compare r6 to zero for a valid image
	MOVS r6, r6	//seem like a nop, but is not. It updates the flags!
	BEQ .L_success	// continue

		// check if the CRC word has the link time constant representing an unverifyable image
		LDR r0, [r5, #-4];	// r5 points one word next to the length
		LDR r1, bl_data_unverifiable_crc_value
		CMP r0, r1
		BEQ .L_unverfiyable

		// return error 3 (InvalidImage)
		MOV r1, r8
		MOV r0, 3
		B .L_ret

	.L_unverfiyable:

	// return error 4 (UnverifyableImage)
	MOV r0, 4
	MOV r1, r8
	B .L_ret

	.L_success:
	MOV r0, 0
	MOV r1, r8	// size

.L_ret:
POP {r4, r5, r6, r7, r8, pc}

bl_data_image_start: .word __image_start
bl_data_update_image_start : .word __update_image_start
bl_data_flashconfig_start: .word __flashconfig_start
bl_data_metadata_offset : .word WD_ABI_METADATA_SECTION_OFFSET
bl_data_metadata_magic: .word WD_ABI_METADATA_MAGIC
bl_data_application_max_size: .word __application_max_size
bl_data_commandword_apply: .word 0x000000FF
bl_data_commandword_success: .word 0x00000000
bl_data_unverifiable_crc_value: .word WD_ABI_UNVERIFIABLE_CRC_VALUE

bl_HardFault_Handler:
	B .
bl_Other_Handler:
	B .

bl_flash_handler:
	MOV pc, pc

// the assembler will emit it's data from the LDR r2, =<constant> expressions at the end of the file
// so this label is to clean up the dissassembly.
bl_misc_data: