#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "list.h"

bool tokenizer(void);
bool parser(void);
bool parsing_operand(int);
bool parse_addressing(int);
bool setstate(bool, bool, bool);
bool generator(void);
bool dcb_process(void);

void format_line(void);
void format_operand(void);
void reset_states(void);

bool preprocessor(const char*);
void assembler(const char*);
void proc_define(void);
void proc_dcb(void);
void (*func_ptr)();

void printerr(const char*);
void printwarn(const char*);
bool recursive_def(char*);

#define MAX_LINE_LENGTH 1024
#define MEMORY_EMULATOR 65535

// -----------------------------------------------------
// Addressing Types
#define NOP		0x00
#define IMM 	0x01
#define ZP 		0x02
#define ZPX 	0x04
#define ZPY 	0x08
#define AB 		0x10
#define ABX		0x20
#define ABY		0x40
#define INDX 	0x80
#define INDY 	0x100
#define IND 	0x200
#define ACC		0x400
#define REL 	0x800
// -----------------------------------------------------

char line[MAX_LINE_LENGTH];
bool isDirective = false;
bool isMnemonic = false;
bool isLiteral = false;
int linenum = 1;
int number;
int len;
char *token;
char *directive;
char *mnemonic;
char *operand;

int mnemonic_index = 0;
int isDefinition = 0;

char dest[50];
char *endptr;

bool isZeroPage = false;
bool isAbsolute = false;
bool isIndirect = false;
bool indirectX = false;
bool indirectY = false;
bool isZeroPageX = false;
bool isZeroPageY = false;
bool isAbsoluteX = false;
bool isAbsoluteY = false;
bool isAccumulator = false;

bool isDecimal = false;

bool toIgnore = false;
bool isLineComment = false;
bool directive_error = false;
int code_index = 0;

unsigned char *memory;
unsigned char *code_address;

DefineList *define_list;
DcbList *dcb_list;

#define MNEMONICS_SIZE 	57
const char* mnemonics[] = {
	"ADC",
	"AND",
	"ASL",
	"BIT",
	
	// Branch Instructions
	"BPL",
	"BMI",
	"BVC",
	"BVS",
	"BCC",
	"BCS",
	"BNE",
	"BEQ",
	
	"BRK",
	
	// Comparators
	"CMP",
	"CPX",
	"CPY",
	
	"DEC",
	"EOR",
	
	// Flag Instructions (Processor Status - No operands)
	"CLC",
	"SEC",
	"CLI",
	"SEI",
	"CLV",
	"CLD",
	"SED",
	
	"INC",
	
	// Jump instructions
	"JMP",
	"JSR",
	
	// Load Instructions
	"LDA",
	"LDX",
	"LDY",
	
	"LSR",
	"NOP",
	"ORA",
	"ROL",
	"ROR",
	"RTI",
	"RTS",
	
	"SBC",
	"STA",
	"STX",
	"STY",
	
	// Stack Instructions
	"PHA",
	"PLA",
	"PHP",
	"PLP",
	"TXS",
	"TSX",
	
	// Register Instructions
	"TAX",
	"TXA",
	"DEX",
	"INX",
	"TAY",
	"TYA",
	"DEY",
	"INY",
	"DCB"
};

#define DIRECTIVES_SIZE 	2
const char* directives[] = {
	"DCB",
	"DEFINE"
};

const char* opcodes[] = {
	0x65, 0x25, 0x06, 0x24, 0x10, 0x30, 0x50, 0x70, 0x90, 0xB0, 0xD0, 0xF0,
	0x00, 0xC5, 0xE4, 0xC4, 0xC6, 0x45, 0x18, 0x38, 0x58, 0x78, 0xB8, 0xD8,
	0xF8, 0xE6, 0x4C, 0x20, 0xA5, 0xA6, 0xA4, 0x46, 0xEA, 0x05, 0x26, 0x66,
	0x40, 0x60, 0xE5, 0x85, 0x86, 0x84, 0x48, 0x68, 0x08, 0x28, 0x9A, 0xBA,
	0xAA, 0x8A, 0xCA, 0xE8, 0xA8, 0x98, 0x88, 0xC8
};

const unsigned short addressing[] = {
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// ADC
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// AND
	ACC + ZP + ZPX + AB + ABX,						// ASL
	ZP + AB,										// BIT
	REL, REL, REL, REL, REL, REL, REL, REL, NOP,	// Branch Instructions & BRK
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// CMP
	IMM + ZP + AB,									// CPX
	IMM + ZP + AB,									// CPY
	ZP + ZPX + AB + ABX, 							// DEC
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// EOR
	NOP, NOP, NOP, NOP, NOP, NOP, NOP,				// Flag Instructions
	ZP + ZPX + AB + ABX, 							// INC
	AB + IND, AB,									// JMP & JSR
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// LDA
	IMM + ZP + ZPY + AB + ABY,						// LDX
	IMM + ZP + ZPX + AB + ABX,						// LDY
	ACC + ZP + ZPX + AB + ABX,						// LSR
	NOP,											// NOP
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// ORA
	ACC + ZP + ZPX + AB + ABX,						// ROL
	ACC + ZP + ZPX + AB + ABX,						// ROR
	NOP, NOP,										// RTI & RTS
	IMM + ZP + ZPX + AB + ABX + ABY + INDX + INDY,	// SBC
	ZP + ZPX + AB + ABX + ABY + INDX + INDY,		// STA
	ZP + ZPY + AB,									// STX
	ZP + ZPX + AB,									// STY
	NOP, NOP, NOP, NOP, NOP, NOP, 					// Stack Instructions
	NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP			// Register Instructions
};

int* process[] = {
	(int*)proc_dcb, (int*)proc_define
};

void printerr(const char* msg){
	printf("Error: Syntax error at line %d - %s.\n", linenum, msg);
}

void printwarn(const char* msg){
	printf("Warning: %s at line %d.\n", msg, linenum);
}

void error(const char* msg){
	printf("%s: error at line %d - %s.\n", mnemonic, linenum, msg);
}

void format_line(){
	line[strcspn(line, "\n")] = '\0';
	for(int i = 0; i < strlen(line); i++){
		if(line[i] == 0x09)
			line[i] = 0x20;
		else if(line[i] > 0x60 && line[i] < 0x7B)
				line[i] -= 0x20;
	}
}

void format_operand(){
	strcat(operand, token);
    token = strtok(NULL, " ");
}

void reset_states(){
	isMnemonic = false;
	isAccumulator = false;
	isLiteral = false;
	isZeroPage = false;
	isZeroPageX = false;
	isZeroPageY = false;
	isAbsolute = false;
	isAbsoluteX = false;
	isAbsoluteY = false;
	isIndirect = false;
	indirectX = false;
	indirectY = false;
	isLineComment = false;
}

void proc_dcb(){
	token = strtok(NULL, " ");
	operand = token;
	token = strtok(NULL, " ");
	while(token != NULL)
		format_operand();
	
	int i = 0;
	int length = 0;
	unsigned char* value = (unsigned char*) malloc(1);
	bool isHexa = false;
	bool isNum = false;
	while(operand[i] != NULL){
		isHexa = operand[i] == '$';
		isNum = operand[i] >= 0x30 && operand[i] <= 0x39;
		if(!isHexa && !isNum && operand[i] != ','){
			directive_error = true;
			printerr("Invalid defined value - use number");
			return;
		}
		if(isHexa || isNum){
			char val[4] = {0};
			int j = 0;
			if(isHexa) i = i + 1;
			while(operand[i] != ',' && operand[i] != NULL)
				val[j++] = operand[i++];
			val[j] = 0;
			
			int num = (isHexa) ? strtol(val, &endptr, 16) : strtol(val, &endptr, 10);
			if((j > 2 && isHexa) || (num > 255 && !isHexa))
				printwarn("DCB byte is larger than 8-bit. Only low byte will be considered");
		
			value[length++] = (unsigned char) num & 0xFF;
			value = (char*) realloc(value, length+1);
			if (*endptr != '\0') {
				directive_error = true;
				printerr("Cannot parse the hexa number");
				return;
			}
			continue;
		}
		i++;
	}
	dcb_list = insertdcb(dcb_list, linenum, length, value);
	free(value);
}

void proc_define(){
	int pos = 1;
	char* name = NULL;
	char* value = NULL;
	
	while(token != NULL){
		token = strtok(NULL, " ");
		switch(pos){
			case 1:	name = token;
					break;
			case 2: value = token;
					break;
			default: if(token != NULL){
						printerr("Invalid defined token");
						directive_error = true;
					 }
		}
		if(directive_error)
			return;
			
		pos++;
	}
	
	bool isNum = name[0] > 0x30 && name[0] <= 0x39;
	bool isSymbol = false;
	for(int i = 0; i < strlen(name); i++)
		isSymbol = name[i] == '#' || name[i] == '$' || name[i] == ':' || isSymbol;
		
	if(isNum || isSymbol){
		printerr("Invalid defined name");
		directive_error = true;
		return;
	}
	if(value[0] == '#'){
		printerr("Invalid defined value - remove '#'");
		directive_error = true;
		return;
	}else if(value[0] == '$'){
		strtol(&value[1], &endptr, 16);
		if (*endptr != '\0') {
			printerr("Invalid defined value - hexa error");
			directive_error = true;
			return;
		}
	}else{
		if(!recursive_def(value)) {
			//++linenum;
			printerr("Undefined value or decimal error");
			directive_error = true;
			return;
		}
	}
	 
	define_list = insertdef(define_list, name, value);
}

bool dcb_process(){
	if(mnemonic_index == 56){
		DcbList* dcb = getdcb(dcb_list, linenum);
		memcpy(&code_address[code_index], dcb->value, dcb->length);
		code_index += dcb->length;
		return true;
	}
	return false;
}

bool recursive_def(char* value){
	int base = (value[0] != '$') ? 10 : 16;
	strtol(&value[base >> 4], &endptr, base);
	if (*endptr != '\0') {
		char* definition = getdef(define_list, value);
		if(definition == NULL)
			return false;
		strcpy(value, definition);
		return recursive_def(value);
	}
	return true;	
}

char* replace(char* token, const char* old_substr, const char* new_substr){
	char* pos;
	int old_len = strlen(old_substr);
	int new_len = strlen(new_substr);
	char* buffer = (char*) malloc(strlen(token) + new_len);
	
	pos = strstr(token, old_substr);
	if(pos != NULL){
		int prefix_len = pos - token;
		strncpy(buffer, token, prefix_len);
		buffer[prefix_len] = '\0';
		
		strcat(buffer, new_substr);
		strcat(buffer, pos + old_len);
		//strcpy(token, buffer);
	}
	return buffer;
}

int check_definition(){
	int index = 0;
	while(token[index] == '#' || token[index] == '$' || token[index] == '(') index++;
	int namelen = strcspn(&token[index], ",");
	char name[namelen+1];
	memcpy(name, &token[index], namelen);
	if(name[namelen-1] == ')'){
		name[namelen-1] = '\0';
		namelen -= 1;
	}
	name[namelen] = 0;
	strtol(&name[0], &endptr, 10);
	if(*endptr != '\0'){
		strtol(&name[0], &endptr, 16);
		if(*endptr != '\0'){
			char* value = getdef(define_list, name);
			if(value == NULL){
				printerr("Undefined value");
				return -1;
			}
			// Fazer substituição aqui...
			//printf("Name Token: '%s', Value Token: %s\n", name, value);
			token = replace(token, name, value);
			//printf("Token modificado: '%s'\n", token);
			return 1;
		}else{
			if((!index && token[index] != '$') || (token[index-1] == '#' && index)){
				char* value = getdef(define_list, name);
				if(value == NULL){
					printerr("Undefined value - possible hexa error (missing prefix)");
					return -1;
				}
				// Fazer substituição aqui...
				//printf("Name Token: '%s', Value Token: %s\n", name, value);
				token = replace(token, name, value);
				//printf("Token modificado: '%s'\n", token);
				return 1;
			}
		}
	}

	return 0;
}

bool preprocessor(const char *filename){
	FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return false;
    }
    
    define_list = begin_def();
    dcb_list = begin_dcb();
    while (fgets(line, sizeof(line), file)){
    	format_line();
		token = strtok(line, " ");
		if(token == NULL || token[0] == ';') continue;
		
		while (token != NULL) {
	    	isLineComment = token[0] == ';' || isLineComment;
	    	if(isLineComment){
	    		token = strtok(NULL, " ");
	    		continue;
			}
			directive_error = false;
			isDirective = true;
			directive = token;
			int i = 0;
			for(; i < DIRECTIVES_SIZE; i++){
				if(strcmp(directives[i], directive) == 0){
					func_ptr = (void(*)())process[i];
					func_ptr();
					break;	
				}	
			}
			if(directive_error)
				return false;
			token = strtok(NULL, " ");
		}
		linenum++;
	}
		
	return true;
}

void assembler(const char *filename) {
	bool isValid = false;
	isValid = preprocessor(filename);
	if(!isValid) return;
		
	memory = (unsigned char *) malloc(MEMORY_EMULATOR * sizeof(unsigned char));
	if (memory == NULL) {
        printf("Erro ao alocar memória.\n");
        return;
    }
    
    // Definir o endereço base de código como 0x600
    code_address = memory + 0x600;
    linenum = 1;
	    
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file)) {
    	
        // Análise Léxica
		isValid = tokenizer();
        if(!isValid)
        	break;
        if(toIgnore){
        	linenum++;
        	continue;
		} 
		
		// Análise sintática
		isValid = parser();
		if(!isValid)
        	break;
        
		// Análise semântica & Gerador...
		isValid = generator();
		if(!isValid)
        	break;
		
		// Liberação de alocação temporária
		if(isDefinition) 
			free(token);
			
        linenum++;
    }

	if(code_index > 2560){
		perror("Error: The maximum program size is 2560 bytes.");
        exit(EXIT_FAILURE);
	}
	
	//if(define_list != NULL)
	//	showdef(define_list);
		
	if(isValid){
		unsigned short address = 0x600;
		printf("Code Length: %d\n", code_index);
		for(int i = 0; i < code_index; i++){
			if(i % 16 == 0)
				printf("\n0x%x: ", address);
			if(code_address[i] < 0x10)
				printf("0%X ", code_address[i]);
			else
				printf("%X ", code_address[i]);
			address++;
		}	
	}
	
    fclose(file);
    if(define_list != NULL) 
		freedef(define_list);
	if(dcb_list != NULL);
		freedcb(dcb_list);
}

bool generator(){
	if(dcb_process())	
		return true;
		
    char opcode = opcodes[mnemonic_index]; // e.g. ADC = $65
    char operand_byte1, operand_byte2;
    bool isBranch = mnemonic_index > 3 && mnemonic_index < 12;
	bool isJump = mnemonic_index == 26 || mnemonic_index == 27;
    
    if(addressing[mnemonic_index] && !isAccumulator){
    	
		operand_byte1 = (char) number & 0xFF;
	    
	    if(isLiteral){
	    	if(addressing[mnemonic_index] & IMM){
		    	switch(mnemonic_index){
		    		case 14:
		    		case 15:
		    		case 29:
		    		case 30:	opcode -= 4;	// e.g. LDY = $A4 - 4 = $A0
		    					break;
		    		default:	opcode += 4;	// e.g. ADC = $65 + 4 = $69
				}	
			}else{
				error("cannot use immediate addressing");
				return false;
			}
	
		}else{
			if(isIndirect){
				if(indirectX){
					if(addressing[mnemonic_index] & INDX)
						opcode -= 4;
					else{
						error("cannot use indirectX addressing");
						return false;
					}
							
				}else if(indirectY){
					if(addressing[mnemonic_index] & INDY)
						opcode += 12;
					else{
						error("cannot use indirectY addressing");
						return false;
					}
						
				}else{
					if(!(addressing[mnemonic_index] & IND)){
						error("cannot use indirect addressing");
						return false;
					}
					if(isJump)	opcode += 32;
				}
			}
			
		    if(isZeroPage && ((isZeroPageX || isZeroPageY) || isBranch)){	//  && mnemonic_index == 29
		    	if(isZeroPageY && !(addressing[mnemonic_index] & ZPY)){
		    			error("cannot use ZeroPageY addressing");
		    			return false;
				}else if(isZeroPageX && !(addressing[mnemonic_index] & ZPX)){
						error("cannot use ZeroPageX addressing");
		    			return false;
		    	}else if(!(addressing[mnemonic_index] & ZP)){
		    		error("cannot use ZeroPage addressing");
		    		return false;
				}else{
		    		opcode += 16;
				}
			}else{
				if(isAbsolute){
					opcode += 8;  	// e.g. ADC = $65 + 8 = $6D
					if(isAbsoluteX){
						if(addressing[mnemonic_index] & ABX)
							opcode += 16;
						else{
							error("cannot use absoluteX addressing");
							return false;
						}
					}else if(isAbsoluteY){
						if(addressing[mnemonic_index] & ABY)
							opcode = (mnemonic_index != 29) ? opcode + 12 : opcode + 16;
						else{
							error("cannot use absoluteY addressing");
							return false;
						}	
					}else{
						if(!(addressing[mnemonic_index] & AB)){
							error("cannot use absolute addressing");
							return false;
						}
						if(isJump)	opcode -= 8;
					}
					
		    		operand_byte2 = (char) ((number & 0xFF00) >> 8);
				}
			}
		}
	    
	    code_address[code_index++] = opcode;
	    code_address[code_index++] = operand_byte1;
	    if(isAbsolute)
	    	code_address[code_index++] = operand_byte2;
	}else{
		if(isAccumulator){
    		if(addressing[mnemonic_index] & ACC)
    			opcode += 4;
    		else{
    			error("cannot use accumulator addressing");
				return false;	
			}
		}
		code_address[code_index++] = opcode;
	}
	return true;
}

bool parser(){
	if(mnemonic_index == 56)
		return true;
	if(!isMnemonic){
		if(!addressing[mnemonic_index]){
			printerr("Instruction expect 0 operand. Given 1");
			return false;
		}
		
		if(isAccumulator)
			return true;
				
		if(isLiteral){
			if(!parsing_operand(2))
        		return false;
        			
			if((len > 2 && !isDecimal) || (number > 255 && isDecimal)){
				printerr("Exceeded the 8-bit limit for literal");
				return false;
			}
				
		}else{	
			if(!parsing_operand(1))
        		return false;
        		
			if((len <= 2 && !isDecimal) || (number < 256 && isDecimal)){
				isZeroPage = true;
			}else if((len < 5 && !isDecimal) || (number < 65536 && isDecimal)){
				isAbsolute = true;
			}else{
				printerr("Exceeded the 16-bit limit for address");
				return false;
			}
		}
	}else{
		isAccumulator = mnemonic_index == 2 || mnemonic_index == 31 || mnemonic_index == 34 || mnemonic_index == 35;
		// Instrução de 0 operando
		if(addressing[mnemonic_index] && !isAccumulator){
			printerr("Instruction expect 1 operand. Given 0");
			return false;
		}
	}
	return true;
}

bool parse_addressing(int index){
		char op[50] = {0};
		bool isParenthesisValid = false;
		int operand_len = strlen(operand);
			
		int count = 0;
		int i = (operand[0] == '(') ? index + 1 : index;
		
		if(isDecimal) --i;
		
			
		/*
		if(isIndirect && operand[index] != '$'){
			printerr("Incorrect indirect number - insert '$'");
			return false;
		}
		*/
		
		for(; i < operand_len; i++){
			if(operand[i] != ',' && operand[i] != ')'){
				if(operand[i] == ';')	
					break;
				op[count++] = operand[i];
			}else{
				if(isLiteral){
					printerr("Incorrect immediate addressing");
					return false;
				}
				if(operand[i] == ')' && isIndirect)
					isParenthesisValid = true;
				else if(operand[i] == ')' && !isIndirect){
						printerr("Missing parenthesis open");
						return false;
					}
				while(++i != operand_len){
					if(operand[i] != ','){
						if(operand[i] == ';')	
							break;
						if(operand[i-1] != ','){
							printerr("Missing comma separator");
							return false;
						}
						
						bool isRegisterX = (operand[i] == 'X' || operand[i] == 'x');
						bool isRegisterY = (operand[i] == 'Y' || operand[i] == 'y');
						bool is8bit = (count <= 2 && !isDecimal) || (atoi(op) < 256 && isDecimal);
						bool is16bit = ((count > 2 && count < 5) && !isDecimal) || ((atoi(op) > 255 && atoi(op) < 65536) && isDecimal);
						indirectX = isRegisterX && is8bit && !isParenthesisValid && isIndirect;
						indirectY = isRegisterY && is8bit && isParenthesisValid && isIndirect;
						isZeroPageX = isRegisterX && is8bit && !isIndirect;
						isZeroPageY = isRegisterY && is8bit && !isIndirect;
						isAbsoluteX = isRegisterX && is16bit && !isIndirect;
						isAbsoluteY = isRegisterY && is16bit && !isIndirect;
						
						if(!indirectY && !indirectX && isIndirect){
							printerr("Invalid indirect addressing");
							return false;
						}
						
						break;
					}
					if(operand[i] == ')' && isIndirect)
						isParenthesisValid = true;
				}
				if(operand[i+1] == ')'){
					if(operand[i+2] != NULL && operand[i+2] != ';'){
						printerr("Invalid operand");
						printf("char error -> %c\n", operand[i+2]);
						return false;
					}	
				}else{
					if(operand[i+1] != NULL && operand[i+1] != ';' && operand[i] != ';'){
						printerr("Invalid operand");
						printf("char error -> %c\n", operand[i+1]);
						return false;
					}
				}
				
				if(operand[i+1] == ')' && !isIndirect){
					printerr("Missing parenthesis open");
					return false;
				}
				if(!isIndirect && (!isZeroPageX && !isZeroPageY && !isAbsoluteX && !isAbsoluteY)){
					printerr("Invalid addressing - Missing register");
					return false;
				}
				bool noJump = (mnemonic_index != 26 && mnemonic_index != 27);
				if(!indirectY && !indirectX && isIndirect && noJump){	// && verificar se é diferente de jumps
					printerr("Invalid indirect addressing");
					return false;
				}
			}
		}
		if(!isParenthesisValid && isIndirect){
			printerr("Missing parenthesis closed");
			return false;
		}
	
	//if(isIndirect)
	//	memcpy(dest, &op[index], count+1);
	//else
		memcpy(dest, op, count+1);
	return true;	
}

bool parsing_operand(int index){
	if(!parse_addressing(index)) return false;
	int base = (isDecimal) ? 10 : 16;
	number = strtol(dest, &endptr, base);
	len = strlen(dest);
	
	if (*endptr != '\0') {
		printerr("Cannot parse the hexa or decimal number");
		return false;
	}
	
	return true;	
}

bool tokenizer(){
	format_line();
	token = strtok(line, " ");
	
	toIgnore = token == NULL || token[0] == ';';
	if(toIgnore) return true;
        
	int i = 0;
	int count_tok = 0;
	reset_states();
	isDefinition = 0;
	
    while (token != NULL) {
    	isLineComment = token[0] == ';' || isLineComment;
    	if(isLineComment){
    		token = strtok(NULL, " ");
    		continue;
		}
    	if(count_tok >= 2){
    		format_operand();
        	count_tok++;
    		continue;
		}
		
		if(strcmp(token, "DEFINE") != 0 && strcmp(token, "DCB") != 0 && count_tok > 0){
			isDefinition = check_definition();
			if(isDefinition == -1)
				return false;
		}
		
			
		if(token[0] == '#'){
			isDecimal = (token[1] != '$');
			if(!setstate(true, false, false)) return false;
		}else if(token[0] == '$'){
				isDecimal = false;
				if(!setstate(false, false, false)) return false;
		}else if(token[0] == '('){
				isDecimal = (token[1] != '$');
				if(!setstate(false, true, false)) return false;
		}else if(strcmp(token, "A") == 0 && isMnemonic){
				isDecimal = false;
				if(!setstate(false, false, true)) return false;
		}else if(token[0] >= 0x30 && token[0] <= 0x39){
				isDecimal = true;
				if(!setstate(false, false, false)) return false;
		}else{
			if(isMnemonic){
				printerr("Invalid operand");
				return false;
			}
			isMnemonic = true;
			mnemonic = token;
			for(; i < MNEMONICS_SIZE; i++)
				if(strcmp(mnemonics[i], mnemonic) == 0)
					break;
			
			toIgnore = strcmp(mnemonic, "DEFINE") == 0;
			if(toIgnore)
				return true;
				
			if(i == MNEMONICS_SIZE){
				printerr("Unknown mnemonic");
				return false;
			}
			
			if(i == 56)
				break;
		}
			
        token = strtok(NULL, " ");
        count_tok++;
    }
    
    mnemonic_index = i;
	return true;	
}

bool setstate(bool literal, bool indirect, bool accumulator){
	if(!isMnemonic){
        printerr("Invalid mnemonic");
        return false;
	}
    isMnemonic = false;
    isLiteral = literal;
    isIndirect = indirect;
    isAccumulator = accumulator;
    operand = token;
    return true;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
        fprintf(stderr, "Usage: %s -m |--mount <arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

	if(strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--mount") == 0)
    	assembler(argv[2]);

    return EXIT_SUCCESS;
}
