define num1 $20
define num2 20
; codigo comentado --------
;adc (num1),Y
;adc (num2,X)
;adc ($10),Y
;adc (10,X)
; -------------------------
adc #num1
adc num1
adc #num2
adc num2
