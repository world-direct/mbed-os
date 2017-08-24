
/*
 * flashconfigasm.s
 *
 * Created: 24.08.2017 21:21:20
 *  Author: Guenter.Prossliner
 */ 

.section .flashconfig_factory,"aw",%progbits
.incbin "flashconfig_factory.txt"
