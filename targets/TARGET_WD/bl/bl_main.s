/*
 * bl_main.s
 *
 * Created: 10.08.2017 11:40:20
 *  Author: Guenter.Prossliner

# Implementation note:

The bootloader is not in a part of the application that is performance-critical.
For sure we should not spend time in doing unnesserary things, but we will focus on maintainability instead of reducing some CPU cycles

# Organization:

The bootloader is seperated into multiple files:

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

#include "bl_asmoptions.inc"
#include "WD_ABI.h"

//////////////////////////////////////////////////////////////////////////
BL_GLOBAL_FUNCTION(bl_start):
//////////////////////////////////////////////////////////////////////////
//	this is the entrypoint of the bootloader

	// B bl_start_app

	/////////////////////////////////////////////////////
	// perform hw initialization
	BL bl_hal_init

	// turn on BUS_LED to signal start of bootloader
	// MED will turn it of on initialization (somewhere in software_init_hook), so you will just see it flashing!
	/////////////////////////////////////////////////////
	MOV r0, #1
	BL bl_hal_ui

	// get bootloader image state
	/////////////////////////////////////////////////////
	BL bl_get_bootloader_image_state
	BL_ASSERT(r0, NE, #0)
										// r1: keystore	
	MOV r4, r1							// r4: keystore
	
	// get application image state
	/////////////////////////////////////////////////////
	LDR r0, bl_data_image_start			// r0: image_start
	MOV r1, r4							// r1: keystore
	BL bl_get_application_image_state
										// r0: application_image_state
	MOV r5, r0							// r5: application_image_state

	// get update image state
	/////////////////////////////////////////////////////
	LDR r0, bl_data_image_start			// r0: image_start
	MOV r1, r4							// r1: keystore
	BL bl_get_application_image_state
										// r0: update_image_state
	MOV r6, r0							// r6: update_image_state


	B bl_start_app	// if (result!=0) bl_start_app();

	// evaluate command-word
	LDR r0, bl_data_update_image_start
	ADD r0, r6		// r0: &command_word
	LDR r0, [r0]	// r0: command_word
	LDR r1, bl_data_commandword_apply
	CMP r1, r0
	BNE bl_start_app	// skip update, because if command-word
	
	MOV r0, r6	//load size
	BL bl_update	// bl_update(size)

	// update command-word to zero
	MOV r0, r6
	ADR r1, bl_data_commandword_success	
	BL bl_set_command_word


//////////////////////////////////////////////////////////////////////////
BL_LOCAL_FUNCTION(bl_start_app):
//	This function starts the application. It will not return.
//////////////////////////////////////////////////////////////////////////

	LDR r0, bl_data_image_start

	// load SP
	LDR r1, [r0, #0]
	MOV sp, r1

	// load PC
	LDR r1, [r0, #4]
	MOV pc, r1


//////////////////////////////////////////////////////////////////////////
BL_LOCAL_FUNCTION(bl_get_application_image_state):
//////////////////////////////////////////////////////////////////////////
//	Reads the application or update flash content to get information about image.
//		r0: void * base_address: (app or update)
//		r1: void * keystore, or null of no DSA 
//
//	{ r0:res } bl_get_application_image_state(void * keystore);
//		r0: return code
//			0: OK (valid, if keystore passed also verified)
//			1: empty (no metadata)
//			2: invalid
//			3: incompatible
//		r1: tail_ptr	// pointer after the image
PUSH {r4, r5, lr}
							// r0: base-address
							// r1: keystore

	MOV r5, r1				// r5: keystore

	// alloc state
	/////////////////////////////////////////////////////
	SUB sp, #0x20			// sp: image-state

	MOV r1, sp				// r1: state
	MOV r2, r5				// r5: keystore
	BL bl_read_image
							// r0: result (0:empty, 1:header exists, 2:has size, 3:crc valid, 4: cpu compatible, 5: dsa validated)

	// if(result==0) return 1	// handle empty
	CMP r0, #0				
	ITT EQ
		MOVEQ r0, 1
		BEQ 0f

	// if(result==3) return 3;	// handle incompatible
	CMP r0, #3			
	ITT EQ
		MOVEQ r0, 3
		BEQ 0f

	// if(keystore)
	CMP r5, #0
	BNE 2f // {
		
		// if(result == 5) return 0
		CMP r0, #5			
		ITT EQ
			MOVEQ r0, #0
			BEQ 0f

		// return 2
		MOV r0, #2
		B 0f
	// }
	2:

	// if(result <= 2) return 2
	CMP r0, #2	
	ITT LS
		MOVLS r0, #2
		BLS 0f

	// return 0
	MOV r0, #2

0:
ADD sp, #0x20
POP {r4, r5, pc}

//////////////////////////////////////////////////////////////////////////
BL_LOCAL_FUNCTION(bl_get_bootloader_image_state):
//////////////////////////////////////////////////////////////////////////
//	Reads the bootloader flash content to get information about the system.
//	Used internally on startup, or if requested by the User by a srvcall
//
//	{ r0:res, r1: keystore } bl_get_bootloader_image_state(void);
//
//		if(res == 0) -> ok
//		if(keystore == 0) -> BL not signed else BL signed
//
//////////////////////////////////////////////////////////////////////////
PUSH {r4, r5, r6, lr}

	// bootloader self verify, without DSA
	/////////////////////////////////////////////////////
	SUB sp, #0x20
	MOV r4, sp				// r4: image-state

	MOV r0, WD_FLASH_BASE	// r0: image base
	MOV r1, r4				// r1: state
	MOV r2, 0				// r2: without keystore in first pass
	BL bl_read_image
							// r0: validation-result

	// return - values
	/////////////////////////////////////////////////////
	MOV r1, 0	// keystore result register defaults to 0

	//	1: DEV-BUILD, no size, no crc, no sig -> continue
	CMP r0, #1
	BEQ .L_bl_dev

	// 4: PROD BUILD: valid, DSA not checked
	CMP r0, #4
	BEQ .L_bl_prod

	// 0: DIE, should not get that far anyway
	// 2: size set, but invalid CTC -> DIE
	// 3: crc ok, cpu incompatible -> DIE	
	MOV r0, 1	// res = 1
	BEQ 0f

	.L_bl_dev:
	MOV r0, 0	// res = 0
	BEQ 0f

	.L_bl_prod:
	MOV r0, 1	// res = 1

	// locate keystore
	LDR r3, [r4, #0x08]				// r3: header-flags
	TST r3, WD_ABI_HDR_FLAG_KEYSTR
	BEQ 0f							// return if no keystore flag

	// calculate offset in r2 from end based on flags
	MOV r2, #WD_ABI_SIZE_KEYSTR
	TST r3, WD_ABI_HDR_FLAG_CRC
	IT NE
	ADDNE r2, #WD_ABI_SIZE_CRC

	TST r3, WD_ABI_HDR_FLAG_DSA
	IT NE
	ADDNE r2, #WD_ABI_SIZE_SIGNATURE

	// get keystore address
	LDR r0, [r4, #0x04]			// r0: image_start
	LDR r1, [r4, #0x0C]			// r1: image_size total
	ADD r0, r1					// r0: image-end
	SUB r0, r2					// r0: &keystore-header

	// test keystore version
	LDR r2, [r0]					// r2: keystore-header
	LDR r1, =#WD_ABI_KEYSTR_MAGIC	// r1: expected header
	CMP r1, r2
	BNE bl_die

	ADD r0, #0x04				// r0: &keystore
	MOV r6, r0					// r6: &keystore

	// re-test with keystore
	MOV r0, WD_FLASH_BASE		// r0: image base
	MOV r1, r4					// r1: state
	MOV r2, r6					// r2: keystore
	BL bl_read_image

	// r0: MUST BE 5 (valid DSA, otherwise we have an invalid SIG!)
	CMP r0, #5
	BNE bl_die

	MOV r0, 0		// r0: 0 (success)
	MOV r1, r6		// r1: keystore
0:
ADD sp, #0x20
POP {r4, r5, r6, pc}

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



//////////////////////////////////////////////////////////////////////////
BL_GLOBAL_FUNCTION(bl_die):
//////////////////////////////////////////////////////////////////////////
//	Notifies about a fatal condition in bootloader.
//	Will not return, need to reset manually.
	0:

		MOV r0, #0
		BL bl_hal_ui
		BL bl_hal_sleep

		MOV r0, #1
		BL bl_hal_ui
		BL bl_hal_sleep

		B 0b



bl_data_image_start: .word __image_start
bl_data_update_image_start : .word __update_image_start
bl_data_metadata_offset : .word WD_ABI_METADATA_SECTION_OFFSET
bl_data_metadata_magic: .word WD_ABI_METADATA_MAGIC
bl_data_application_max_size: .word __application_max_size
bl_data_commandword_apply: .word 0x000000FF
bl_data_commandword_success: .word 0x00000000
// TODO //
bl_data_unverifiable_crc_value: .word 0
bl_data_cpuid_address: .word 0xE000ED00


// the assembler will emit it's data from the LDR r2, =<constant> expressions at the end of the file
// so this label is to clean up the dissassembly.
bl_misc_data: