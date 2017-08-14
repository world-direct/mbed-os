
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

	// turn on the BUS-LED (PE10 = 0x4A)
	// port = 0x4A >> 4 =  0x4 GPIOE_BASE = 0x400210000

	// pin = 0x4A & 0xF = 0xA
	// init-mask = 0xA*2 = 1 << 0xA * 2 (2 bits wide) = 100000
	// on-value-mask = 1 << 0xA = 400 


	// test some "none failable" instructions
	MOV r0, #1
	MOV r1, #255


	// enable output on BUS-LED
	/*LDR r3, =0x40210000
	LDR r2, =0x100000
	STR r2, [r2]

	// and turn it on
	*/

	// load SP
	LDR r3, =0x08004000
	MOV sp, r3

	// jump to app
	LDR r3, =0x08004004
	MOV pc, r3

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
