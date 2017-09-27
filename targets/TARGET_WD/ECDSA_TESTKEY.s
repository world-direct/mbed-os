/*
	THIS IS FOR DEBUGGING THE BOOTLOADER DSA IMPLEMENTATION ONLY.

	Generated: 26.09.2017 13:25:06
	Key-File: .\testkey.pem commited in the vNextElfTool
	Key-Thumbprint: E1-38-51-D3-34-77-3A-80-11-0B-37-A4-34-61-D3-CB-87-AD-CC-9E-07-7A-32-23-AE-AE-CB-39-CD-EA-E0-19
	Endianess: Big
	Word-Order: Reversed

	This Key is always compiled into the .elf, but is is not active, until WD_ABI_BL_HEADER_FLAG_PRODUCTION header flag is set
*/

.section .bl_ecdsa_testkey,"ax",%progbits

#define BL_ECDSA_THUMBPRINT 0xCDEAE019, 0xAEAECB39, 0x077A3223, 0x87ADCC9E, 0x3461D3CB, 0x110B37A4, 0x34773A80, 0xE13851D3
#define BL_ECDSA_PUBX 0xA14DDD6C, 0x43F9223C, 0x69177D35, 0xA900FC19, 0xE6F81304, 0x4A6456DE, 0x23137AD5, 0x5EF8A2D3
#define BL_ECDSA_PUBY 0x1B44945A, 0x2EFEED18, 0xE35EC97F, 0x2EF16341, 0x6203D844, 0x184D0AFA, 0x6BD934AD, 0xBFE8A50F

	.word 0xFFFFFFFF	// this is just to avoid, that if the test-key get linked directly after the bootloader, that this can be misinterpreted as a bootloader 

	.word 0x00000001	// DSA Version
	.word BL_ECDSA_THUMBPRINT
	.word BL_ECDSA_PUBX
	.word BL_ECDSA_PUBY

