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

#define WD_ABI_BL_SIZE_OFFSET	0x204

#define WD_ABI_METADATA_SECTION_OFFSET	0x300
#define WD_ABI_METADATA_MAGIC			0x01020304
#define WD_ABI_UNVERIFIABLE_CRC_VALUE	0xF666777F

#define WD_ABI_APPLICATION_NAME_SIZE	0x20
#define WD_ABI_APPLICATION_VERSION_SIZE	0x20

#endif /* WD_ABI_H_ */