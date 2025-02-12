; codigo comentado --------
;adc (num1),Y
;adc (num2,X)
;adc ($10),Y
;adc (10,X)
;adc #num1
;adc num1
;adc #num2
;adc num2
; -------------------------
define num1 $20
define num2 20
adc num2
adc 20,x
adc #num2
adc #20
adc num1
adc $20
adc #num1
adc #$20
adc num2,x
adc 20,x
adc num1,x
adc $20,x
adc (num2,x)
adc (num1),y
adc ( num1,x )
