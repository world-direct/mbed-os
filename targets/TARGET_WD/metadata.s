#include "WD_ABI.h"
#define EXPSTR(a) EXPSTRHLP(a)
#define EXPSTRHLP(a) #a

.section .metadata,"a",%progbits
	. = 0;												.word WD_ABI_METADATA_MAGIC
	. = WD_ABI_METADATA_FLDOFF_SIZE;					.word __image_size
	. = WD_ABI_METADATA_FLDOFF_APPLICATION_NAME;		.asciz EXPSTR(APPLICATION_NAME)
	. = WD_ABI_METADATA_FLDOFF_APPLICATION_VERSION;		.asciz EXPSTR(APPLICATION_VERSION)
	. = WD_ABI_METADATA_FLDOFF_CPUID_MASK;				.word WD_ABI_CPUID_MASK
	. = WD_ABI_METADATA_FLDOFF_CPUID;					.word WD_ABI_CPUID

.section .crc32,"a",%progbits
	.word WD_ABI_UNVERIFIABLE_CRC_VALUE		// this will be linked at the end of FLASH, and patched in the elf file to the correct crc value
