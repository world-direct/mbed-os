
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




.syntax unified
.thumb

.section  .bl_vectors,"a",%progbits
.type  g_bl_vectors, %object
.size  g_bl_vectors, .-g_bl_vectors

g_bl_vectors:

	.word 0x20001000		// SP for bootloader
	.word bl_start	// reset vector
	.word bl_NMI_Handler
	.word bl_HardFault_Handler
	.word bl_MemManage_Handler
	.word bl_BusFault_Handler
	.word bl_UsageFault_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word bl_SVC_Handler
	.word bl_DebugMon_Handler
	.word 0
	.word bl_PendSV_Handler
	.word bl_SysTick_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word bl_flash_handler

.section .bl_text,"ax",%progbits


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
	MOV r6, r1
	MOVS r5, r0
	
	// valid?
	ITT EQ
	MOVEQ r0, r6
	BLEQ bl_update

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
	void bl_update(void* endptr)
	this is the entypoint for the sw-update
*/
.global bl_update
.type bl_update, %function
bl_update:
PUSH {lr}

	// variable regisers
	// r4: endptr
	MOV r4, r0

	// load command word
	LDR r0, [r4]
	BNE 0f	// return if != 0

	// erase app bank
	BL bl_hal_erase_boot_image

0:
POP {pc}


/*************************************************************************
	struct {
		int status; // returned in r0
		void * endptr; // returned in r1
	} bl_validate_image(void*)

	The endptr is only valid if status == 0
	The endptr contains the very next word after the image incl. crc

	This method performs the image validation for the given start-address (which is bl_data_image_start or bl_data_update_image_start)
	It returns one of the following enumation values

	0: ValidImage
	1: NoMetadata (Signature != 0x01020304)
	2: InvalidMetadata (Length > max)
	3: InvalidImage (CRC validation failed)
*/
.type bl_validate_image, %function
bl_validate_image:
PUSH {r4, r5, r6, r7, lr}

	////////////////////////////
	// local constant vars ;-)
	MOV r4, r0	// r4 will hold the base-address
				// r7 will be initialized later for the image-size

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
	LDR r7, [r5, #4]	// len has offset of 4, see metadata.s, r7 will be used in .L_validate_image!

	// load comparant to r6
	LDR r6, bl_data_application_max_size

	// and do the compare
	CMP r6, r7
	BHI .L_validate_image	// continue
		// return error 2 (InvalidMetadata)
		MOV r0, 2
		B .L_ret

.L_validate_image:

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

	// check if we are done. SUB updates the flags, so ne need to compare here
	SUBS r7, #4
	BNE .L_validate_next_word

	// compare r6 to zero for a valid image
	MOVS r6, r6	//seem like a nop, but is not. It updates the flags!
	BEQ .L_success	// continue
		// return error 3 (InvalidImage)
		MOV r0, 3
		B .L_ret

	.L_success:
	MOV r0, 0
	MOV r1, r5	// endptr

.L_ret:
POP {r4, r5, r6, r7, pc}

bl_data_image_start: .word __image_start
bl_data_update_image_start : .word __update_image_start
bl_data_metadata_offset : .word __metadata_offset
bl_data_metadata_magic: .word __metadata_magic
bl_data_application_max_size: .word __application_max_size

bl_NMI_Handler:
	MOV pc, pc
bl_HardFault_Handler:
	MOV pc, pc
bl_MemManage_Handler:
	MOV pc, pc
bl_BusFault_Handler:
	MOV pc, pc
bl_UsageFault_Handler:
	MOV pc, pc
bl_SVC_Handler:
	MOV pc, pc
bl_DebugMon_Handler:
	MOV pc, pc
bl_PendSV_Handler:
	MOV pc, pc
bl_SysTick_Handler:
	MOV pc, pc

bl_flash_handler:
	MOV pc, pc

// the assembler will emit it's data from the LDR r2, =<constant> expressions at the end of the file
// so this label is to clean up the dissassembly.
bl_misc_data: