define str1 String1
define str2 String2

String1:
	dcb $48, $65, $6C, $6F

String2:
	dcb $4F, $6C, $61
	
Vector:
	dcb String1, String2
