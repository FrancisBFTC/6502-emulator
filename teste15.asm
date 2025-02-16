define label5 $000
define label6 $000
nop
label1:
	lda dados
	jmp label2
	nop
	nop
label2 :
	jsr label3
	nop
	nop
label3	:
	jmp label4
	nop
	nop
label4:
	nop
	nop
	jsr label1
	
dados:
	DCB $FF, $FF, $FF
