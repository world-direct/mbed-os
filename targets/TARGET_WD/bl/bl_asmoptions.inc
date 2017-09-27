/*
 * bl_asmoptions.inc
 *
 * Created: 27.09.2017 14:35:30
 *  Author: Guenter.Prossliner
 */ 


 #define BL_ECDSA_DEBUG	1


.syntax unified
.thumb
#include "bl_asmoptions_targetspecific.inc"

#define BL_GLOBAL_FUNCTION(name) \
	.type name, %function;	\
	.global name;		\
	name

#define BL_LOCAL_FUNCTION(name)	\
	.type name, %function;	\
	name


#if (BL_ECDSA_DEBUG) 
#	define BL_FUNCTION(name) BL_GLOBAL_FUNCTION(name)
#else
#	define BL_FUNCTION(name) BL_LOCAL_FUNCTION(name)
#endif


// we use .bl_text as default, it may get redefined if needed in the .S file
.section .bl_text,"ax",%progbits
