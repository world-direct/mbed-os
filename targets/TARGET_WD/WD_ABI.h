/*
 * WD_ABI.h
 *
 * Created: 28.08.2017 14:42:49
 *  Author: Guenter.Prossliner


	IMPORTANT NOTE:
	This Header file defines the constants specified for the ABI (Application Binary Interface)
	used to support the Metadata and Bootloader for World-Direct applications.

	=> THESE VALUES CANNOT BE CHANGED WITHOUT BREAKING THIS INTERFACE
	=> THESE VALUES ARE ALSO CODED INTO EXTERNAL TOOLS (SERVER AND BUILD)


 */ 


#ifndef WD_ABI_H_
#define WD_ABI_H_

#define  WD_FLASH_BASE	0x08000000

#define WD_ABI_BL_HEADER_OFFSET				0x200
#define WD_ABI_BL_HEADER_FLDOFF_SRVCALL		0x04
#define WD_ABI_BL_HEADER_FLDOFF_LENGTH		0x08	// Length = total section length
#define WD_ABI_BL_HEADER_FLDOFF_SIZE		0x0C	// Size = consumed size incl. CRC word
#define WD_ABI_BL_HEADER_FLDOFF_BANK2OFFSET	0x10
#define WD_ABI_BL_HEADER_FLDOFF_KEYSTORE	0x14

#define WD_ABI_BL_HEADER_MAGIC				0x0102	// this is placed in the high halfword of the flags

// the following declarations are for BL-HEADER v1 (Magic = 0102)
#define WD_ABI_BL_HEADER_FLAG_PRODUCTION	0x01	// only valid images are accepted by the bootloader in general, if there is a DSAKEY they have to be signed

#define WD_ABI_METADATA_SECTION_OFFSET		0x300
#define WD_ABI_METADATA_MAGIC				0x01020304
#define WD_ABI_UNVERIFIABLE_CRC_VALUE		0xF666777F
#define WD_ABI_KEYSTORE_CURRENT_VERSION		0xCC000001
#define WD_ABI_SIGNATURE_CURRENT_VERSION	0x01000040

// these constants specify the offsets of the fields in the .metadata section
#define WD_ABI_METADATA_FLDOFF_SIZE					0x04
#define WD_ABI_METADATA_FLDOFF_APPLICATION_NAME		0x08
#define WD_ABI_METADATA_FLDOFF_APPLICATION_VERSION	0x28
#define WD_ABI_METADATA_FLDOFF_CPUID_MASK			0x48
#define WD_ABI_METADATA_FLDOFF_CPUID				0x4C

#define WD_ABI_APPLICATION_NAME_SIZE	(WD_ABI_METADATA_FLDOFF_APPLICATION_VERSION-WD_ABI_METADATA_FLDOFF_APPLICATION_NAME)
#define WD_ABI_APPLICATION_VERSION_SIZE	(WD_ABI_METADATA_FLDOFF_CPUID_MASK-WD_ABI_METADATA_FLDOFF_APPLICATION_VERSION)

#endif /* WD_ABI_H_ */