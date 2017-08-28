.syntax unified
.section .metadata,"a",%progbits
.cpu cortex-m4
.fpu softvfp
.thumb

#define EXPSTR(a) EXPSTRHLP(a)
#define EXPSTRHLP(a) #a

.global  g_metadata

.g_metadata:
.word __metadata_magic	/* metadata magic from .ld script (we dont use just 1, so that we can dedect endianess) */
.word __image_size		/* this field may get rewritten by the platform server, to really match the size (we have current 0x10 bytes missing, maybe from uvisor?) */

.macro padded_string string, max
1:
    .ascii "\string"
2:
    .iflt \max - (2b - 1b)
    .error "String too long"
    .endif

    .ifgt \max - (2b - 1b)
    .zero \max - (2b - 1b)
    .endif

.endm


padded_string EXPSTR(APPLICATION_NAME), 32
padded_string EXPSTR(APPLICATION_VERSION), 16

#define HEXLH(S) 0x ## S
#define HEXL(S) HEXLH(S)

.octa HEXL(APPLICATION_COMMIT_ID)


.section .crc32,"a",%progbits
.word 0xFFFFFFFF	// this will be linked at the end of FLASH, and patched in the elf file to the correct crc value

