define DEC 255
define A $fe
define B 1
define C 254
nop
label1:
	bpl label1
	bcc label2
	bne label1
	nop
	bne label1
label2:
	nop
.BYTE A, B, C, DEC, 0,0,0
