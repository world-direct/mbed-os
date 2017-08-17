
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
	LDR r3, bl_hal_rcc_ahb1enr_address	// use r3 for register address
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
	LDR r3, bl_hal_gpioe_address
	MOV r2, 0x100000
	STR r2, [r3, #0]

	// and turn it on
	MOV r2, #0x400
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


/*************************************************************************
	void bl_hal_flash_unlock():
	
	Unlocks flash controller if not yet done 
*/
.global bl_hal_flash_unlock
.type bl_hal_flash_unlock, %function
bl_hal_flash_unlock:

PUSH {lr}

	BL bl_hal_flash_wait_idle
	
	LDR r1, bl_hal_flashc_address
	LDR r0, [r1, #0x10]		// FLASH_CR
	ANDS r0, #0x80000000
	BEQ 0f	// return if true

	// unlock sequence
	LDR r0, bl_hal_flash_key1
	STR r0, [r1, #0x04] // FLASH_KEYR
	LDR r0, bl_hal_flash_key2
	STR r0, [r1, #0x04] // FLASH_KEYR

0:
POP {pc}

/*************************************************************************
	void bl_hal_erase_sector(int sector_nr):
	
	Erases the specified sector
*/
.type bl_hal_erase_sector, %function
bl_hal_erase_sector:

PUSH {lr}

	// construct value of FLASH_CR
	MOV r2, r0, LSL #3 // sector number bit 3-6

	ORR r2, #0x10000	// STRT: bit 16
	ORR r2, #0x200 // PSIZE=b10 : bit 89
	ORR r2, #0x3 // SER and PG : bit1 and bit 0
	ORR r2, #0x2000000	// ERRIE: bit 25

	BL bl_hal_flash_unlock
	BL bl_hal_flash_wait_idle

	LDR r1, bl_hal_flashc_address
	STR r2, [r1, #0x10]
0:
POP {pc}

/*************************************************************************
	void bl_hal_erase_boot_image(void):
	
	Erases the whole boot-image
*/
.global bl_hal_erase_boot_image
.type bl_hal_erase_boot_image, %function
bl_hal_erase_boot_image:
PUSH {r4, r5, lr}

	MOV r4, #1	// start and current sector
	MOV r5, #11  // last sector

	1:
	MOV r0, r4
	BL bl_hal_erase_sector
	ADD r4, #1
	CMP r4, r5
	BNE 1b


0:
POP {r4, r5, pc}

/*************************************************************************
	void bl_hal_flash_wait_idle():
	
	Polls to the FLASH_SR to wait for an operation to finish
*/
.type bl_hal_flash_wait_idle, %function
bl_hal_flash_wait_idle:

PUSH {lr}
	
	LDR r1, bl_hal_flashc_address

1:
	LDR r0, [r1, #0x0C]		// FLASH_SR
	ANDS r0, #0x00010000		// bit 16: BSY
	BNE 1b	// retest


POP {pc}


bl_hal_gpioe_address:	.word 0x40021000
bl_hal_rcc_ahb1enr_address: .word 0x40023830
bl_hal_crc_address:		.word 0x40023000
bl_hal_flashc_address:	.word 0x40023C00
bl_hal_flash_key1:		.word 0x45670123
bl_hal_flash_key2:		.word 0xCDEF89AB



// the assembler will emit it's data from the LDR r2, =<constant> expressions at the end of the file
// so this label is to clean up the dissassembly.
bl_hal_misc_data: