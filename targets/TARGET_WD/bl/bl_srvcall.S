/*
 * bl_srvcall.S
 *
 * Created: 02.11.2017 14:20:46
 *  Author: Guenter.Prossliner
 */ 

#include "bl_asmoptions.inc"

//////////////////////////////////////////////////////////////////////////
BL_GLOBAL_FUNCTION(bl_srv_call):
//////////////////////////////////////////////////////////////////////////
//	This is the entrypoint service calls from the application to the bootloader.
//	it's it called indirectly from the application by the address emitted to ABI Header Offset SRVCALL
//		r0: struct blsrv_desc * arg (see blsrv.h)
PUSH {r4, r5, r6, lr}

	MOV r4, r0					// r4: arg, will be post-incremented to read struct in stream
	LDR r1, [r4], #4			// r1: operation
	
	SUB r1, #1					// r1: operation table offset (commands start with 1, vs table with 0)
	CMP r1, #5					// 5 is is the highest accepted offset
	ITT HI						// if (r1>5) return false;
	MOVHI r0, 0
	BHI 0f

	ADR r0, bl_srv_call_jump_table	// r0: jump table address
	LDR r1, [r0, r1, LSL #2]		// r1: label address (pc=[r0+(r1<<2)])
	ORR r1, #1						// thumb bit :-(
	BX r1

	// r5 and r6 are alread pushed. They can be used to store vars in the case blocks (.L_blsrv_*)
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

		// BL bl_validate_image // r0=ret_code, r1=size
		MOV r6, r1	// store size

		// output fields:
		STR r0, [r4], #4	// validation_result

		MOV r1, r5
		//LDR r2, bl_data_metadata_offset
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
		// BL bl_validate_image
		MOV r6, r1	// size
		MOVS r5, r0	// result

		// set validation result to output struct field
		STR r5, [r4], #4	

		BNE 1f	// if (result!=0) return

		MOV r0, r6
		//ADR r1,  bl_data_commandword_apply
		MOV r1, 0
		//BL bl_set_command_word

		B 1f

	.L_blsrv_write_config_data:
		// load args
		LDR r0, [r4], #4	// load offset
		MOV r1, #0x210	// BL ABI Header (BANK2OFF)
		LDR r1, [r1]	// load base address 
		ADD r0, r1			// dest = offset + base address
		LDR r1, [r4], #4	// src
		LDR r2, [r4], #4	// size
		BL bl_hal_flash_memcpy
		B 1f

1:
MOVS r0, 1
0:
POP {r4, r5, r6, pc}

bl_srv_call_jump_table:
.align 4
.word .L_blsrv_erase_update_region		// 1
.word .L_blsrv_write_update_region		// 2
.word .L_blsrv_validate_update_image	// 3
.word .L_blsrv_validate_boot_image		// 4
.word .L_blsrv_apply_update				// 5
.word .L_blsrv_write_config_data		// 6


bl_data_image_start: .word __image_start
bl_data_update_image_start : .word __update_image_start
