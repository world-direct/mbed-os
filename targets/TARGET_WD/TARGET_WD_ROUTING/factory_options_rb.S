/*
 * factory_options_rb.S
 *
 * Created: 11.09.2017 11:35:32
 *  Author: Guenter.Prossliner
 */ 

// this section is only used for the vNextElfTool, to extract the address and value of fuses to be programmed in the factory.hex
// this data is not included in the image
.section .factory_options,"a",%note
.word 0x1FFFF808	// option address of 
.word 0x00FF01FE	// option value
