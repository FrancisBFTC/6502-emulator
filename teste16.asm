nop
label1:
	bpl label1
	bcc label2
	bne label1
	nop
	bne label1
label2:
	nop
.BYTE 255, 254, 253
