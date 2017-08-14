
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

	// turn on BUS_LED to signal start of bootloader
	// MED will turn it of on initialization (somewhere in software_init_hook), so you will just see it flashing!
	/////////////////////////////////////////////////////

	// start GPIOE Clk
	// we need to set bit #4 (GPIOEEN) on the RCC_AHB1ENR register (@40023830)
	// this register has a default value of != 0, so we need to read it
	LDR r3, =0x40023830	// use r3 for register address
	LDR r2, [r3]		// read current value into r2 (should be 0x00100000 as documented)
	ORR r2, r2, #0x10	// perform OR
	STR r2, [r3, #0]	// and write to register

	// turn on the BUS-LED (PE10 = 0x4A)
	// port = 0x4A >> 4 =  0x4 GPIOE_BASE = 0x40021000

	// pin = 0x4A & 0xF = 0xA
	// init-mask = 0xA*2 = 1 << 0xA * 2 (2 bits wide) = 100000
	// on-value-mask = 1 << 0xA = 400 

	// enable output on BUS-LED
	// *((volatile int *)0x40210000) = 0x100000;
	LDR r3, =0x40021000
	LDR r2, =0x100000
	STR r2, [r3, #0]

	// and turn it on
	LDR r2, =0x400
	STR r2, [r3, #0x14]

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
