DEFINE NAME $01
DEFINE AGE 28
DCB 1,$02,$102,258
DCB $02,$00A,$04

adc 	$44
sbc 	$44,x
jmp 	$a000
 
nop
; Comentario 1
nop
; Comentario 2
nop
adc 	$44		; Comentário 1
sbc 	$44, x	; Comentário 2
jmp 	$a000	; Comentário 3

adc 	$44;Comentário 1
sbc 	$44,x;Comentário 2
sbc 	($55,X);Comentário 3
; Fim de linha
;
