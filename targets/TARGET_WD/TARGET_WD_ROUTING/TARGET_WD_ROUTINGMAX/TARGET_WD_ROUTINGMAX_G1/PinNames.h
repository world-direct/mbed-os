/* mbed Microcontroller Library
 *******************************************************************************
 * Copyright (c) 2016, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */
#ifndef MBED_PINNAMES_H
#define MBED_PINNAMES_H

#include "cmsis.h"
#include "PinNamesTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PA_0  = 0x00,
    PA_1  = 0x01,
    PA_2  = 0x02,
    PA_3  = 0x03,
    PA_4  = 0x04,
    PA_5  = 0x05,
    PA_6  = 0x06,
    PA_7  = 0x07,
    PA_8  = 0x08,
    PA_9  = 0x09,
    PA_10 = 0x0A,
    PA_11 = 0x0B,
    PA_12 = 0x0C,
    PA_13 = 0x0D,
    PA_14 = 0x0E,
    PA_15 = 0x0F,

    PB_0  = 0x10,
    PB_1  = 0x11,
    PB_2  = 0x12,
    PB_3  = 0x13,
    PB_4  = 0x14,
    PB_5  = 0x15,
    PB_6  = 0x16,
    PB_7  = 0x17,
    PB_8  = 0x18,
    PB_9  = 0x19,
    PB_10 = 0x1A,
    PB_11 = 0x1B,
    PB_12 = 0x1C,
    PB_13 = 0x1D,
    PB_14 = 0x1E,
    PB_15 = 0x1F,

    PC_0  = 0x20,
    PC_1  = 0x21,
    PC_2  = 0x22,
    PC_3  = 0x23,
    PC_4  = 0x24,
    PC_5  = 0x25,
    PC_6  = 0x26,
    PC_7  = 0x27,
    PC_8  = 0x28,
    PC_9  = 0x29,
    PC_10 = 0x2A,
    PC_11 = 0x2B,
    PC_12 = 0x2C,
    PC_13 = 0x2D,
    PC_14 = 0x2E,
    PC_15 = 0x2F,

    PD_0  = 0x30,
    PD_1  = 0x31,
    PD_2  = 0x32,

	// Generic signals namings
    SERIAL_TX   = PC_10, // Virtual Com Port
    SERIAL_RX   = PC_11, // Virtual Com Port
    USBTX       = PC_10, // Virtual Com Port
    USBRX       = PC_11, // Virtual Com Port
	
	SPI_MOSI    = PB_15,
    SPI_MISO    = PB_14,
    SPI_SCK     = PB_13,
    SPI_CS0     = PC_7,	
	SPI_CS1     = PC_6,		
	SPI_CS2		= PA_0,
	SPI_CS3		= PC_9,
	SPI_CS4		= PC_8,
	SPI_CS5		= PA_5,
	
	SPI_CS_FLASH	= SPI_CS2, // was PB_2
	
	BUS_nINT	= PC_0,
	
	In1			= PC_14,
	In2			= PC_15,
	In3			= PA_8,
	In4			= PA_9,
	In5			= PA_10,
	In6			= PB_5,	// was PB_9
	In7			= PC_13, // was PC_8
	In8			= PB_8,	// was PC_9
	In9			= PB_2,	// was PA_0
	In10		= PA_4,
	
	Out1		= PC_3,
	Out2		= PC_2,
	Out3		= PC_1,
	
	Version1	= PA_6,
	Version2	= PA_7,
	Version4	= PC_4,
	Version8	= PC_5,
	Version16	= PB_0,
	
	MUX_0		= PB_10,
	MUX_1		= PB_11,
	MUX_2		= PB_12,
	
	ONEWIRE_TxH	= PB_4,
	ONEWIRE_Tx	= PD_2,
	ONEWIRE_Rx	= PC_12,
	
	Tx0			= PB_6,	// BUS Tx
	Rx0			= PB_7,	// BUS Rx
	
	BUS_LED		= PB_8,
	LED1		= BUS_LED,
	
	USB_ID		= PA_15,
	
	RS485_En	= PA_1,
	RS485_Tx1	= PA_2,
	RS485_Rx1	= PA_3,
	
    // Not connected
    NC = (int)0xFFFFFFFF
} PinName;

#ifdef __cplusplus
}
#endif

#endif
