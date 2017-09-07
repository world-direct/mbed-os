.syntax unified
.section .metadata,"a",%progbits

#include "WD_ABI.h"

#define EXPSTR(a) EXPSTRHLP(a)
#define EXPSTRHLP(a) #a

.word WD_ABI_METADATA_MAGIC	/* metadata magic from .ld script (we dont use just 1, so that we can dedect endianess) */

. = WD_ABI_METADATA_FLDOFF_SIZE
.word __image_size		/* this field may gets includes the CRC sum. Is is validated by the platform server */

. = WD_ABI_METADATA_FLDOFF_APPLICATION_NAME
.ascii EXPSTR(APPLICATION_NAME)

. = WD_ABI_METADATA_FLDOFF_APPLICATION_VERSION
.ascii EXPSTR(APPLICATION_VERSION)

. = WD_ABI_METADATA_FLDOFF_CPUID_MASK
.word WD_ABI_CPUID_MASK

. = WD_ABI_METADATA_FLDOFF_CPUID
.word WD_ABI_CPUID

.section .crc32,"a",%progbits
.word WD_ABI_UNVERIFIABLE_CRC_VALUE	// this will be linked at the end of FLASH, and patched in the elf file to the correct crc value

