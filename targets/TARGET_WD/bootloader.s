
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

 */ 




.syntax unified
.cpu cortex-m4
.thumb

.section  .bl_vectors,"a",%progbits
.type  g_bl_vectors, %object
.size  g_bl_vectors, .-g_bl_vectors

g_bl_vectors:

	.word __stack		// SP
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

.section .bl_text,"ax",%progbits


/*************************************************************************
	void bl_start(void)
	this is the entrypoint of the bootloader
*/
.global bl_start
.type bl_start, %function
bl_start:

	BL bl_hal_init
	BL bl_hal_ui

	// start application
	/////////////////////////////////////////////
	LDR r0, =__image_start

	// load SP
	LDR r1, [r0, #0]
	MOV sp, r1

	// load PC
	LDR r1, [r0, #4]
	MOV pc, r1


bl_data:
	// bring in some data
	.word __image_start

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
