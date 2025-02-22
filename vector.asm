define str1 String1
define str2 String2

Vector:
	dcb str1, <str2
	
String1:
	dcb $48, $65, $6C, $6F

String2:
	dcb $4F, $6C, $61
