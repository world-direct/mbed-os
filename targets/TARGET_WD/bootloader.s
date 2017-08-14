
/*
 * bootloader.s
 *
 * Created: 10.08.2017 11:40:20
 *  Author: Guenter.Prossliner
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

.global bl_start
.type bl_start, %function
bl_start:

	BL bl_hal_init

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
