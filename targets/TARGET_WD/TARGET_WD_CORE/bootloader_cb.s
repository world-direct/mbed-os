
/*
 * bootloader_cb.s
 *
 * Created: 14.08.2017 14:37:44
 *  Author: Guenter.Prossliner
 */ 

.syntax unified
.cpu cortex-m4
.thumb

.section .bl_text,"ax",%progbits

.global bl_hal_init
.type bl_hal_init, %function
bl_hal_init:

	/////////////////////////////////////////////////////
	// CLOCKS:
	// 
	// first we need to enable the clocks needed in bootloader in AHB1 bus
	//	GPIOE (for bus-led): bit #4 (GPIOEEN) on the RCC_AHB1ENR register (@40023830)
	//  CRC (for validation): bit #12 (CRCEN) also on the RCC_AHB1ENR register (@40023830)

	// set enable bits in RCC_AHB1ENR register (@40023830)
	// this register has a default value of != 0, so we need to read it
	LDR r3, =0x40023830	// use r3 for register address
	LDR r2, [r3]		// read current value into r2 (should be 0x00100000 as documented)
	LDR r1, =#0x1010	// the mask
	ORR r2, r2, r1	// perform OR
	STR r2, [r3, #0]	// and write to register


	// turn on BUS_LED to signal start of bootloader
	// MED will turn it of on initialization (somewhere in software_init_hook), so you will just see it flashing!
	/////////////////////////////////////////////////////

	// start GPIOE Clk

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

	MOV pc, lr
