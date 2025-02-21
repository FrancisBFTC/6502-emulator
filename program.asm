define index $02

Start:
	ldx #index
	jsr ReadIndex
	brk
	
ReadIndex:
	lda Array,X
rts

nop
nop
Array:
	dcb $03, $06, $09
	
