
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

/*************************************************************************
	void bl_hal_init(void):
	Initializes the HW resources needed for other functions in the hal
*/
.global bl_hal_init
.type bl_hal_init, %function
bl_hal_init:

PUSH {lr}

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


POP {pc}

/*************************************************************************
	void bl_hal_ui(void):
	Signals the user about the running bootloader (by BUS_LED)
*/
.global bl_hal_ui
.type bl_hal_ui, %function
bl_hal_ui:

PUSH {lr}


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

POP {pc}

/*************************************************************************
	void bl_hal_crc_init(void):
	Initializes the crc generator to the init value of 0xFFFFFFFF
*/
.global bl_hal_crc_init
.type bl_hal_crc_init, %function
bl_hal_crc_init:

PUSH {lr}
	
	MOV r1, #1
	LDR r2, bl_hal_crc_address		// CRC_ base address
	STR r1, [r2, #0x8]				// CRC_CR, set bit #0 (RESET) which clears the DR

POP {pc}

/*************************************************************************
	unsigned int bl_hal_crc_update(unsigned int):
	Updates the crc calculation 
*/
.global bl_hal_crc_update
.type bl_hal_crc_update, %function
bl_hal_crc_update:

PUSH {lr}
	
	LDR r2, bl_hal_crc_address
	STR r0, [r2, #0]		// CRC_DR, we have the argument passed in r0
	LDR r0, [r2, #0]		// read the updated value and return it in r0

POP {pc}

bl_hal_crc_address:	.word 0x40023000


// the assembler will emit it's data from the LDR r2, =<constant> expressions at the end of the file
// so this label is to clean up the dissassembly.
bl_hal_misc_data: