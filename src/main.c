#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool tokenizer(void);
bool parser(void);
bool parsing_operand(int);
bool parse_addressing(int);
bool setstate(bool, bool);
void generator(void);

void format_line(void);
void format_operand(void);

#define MAX_LINE_LENGTH 1024
#define MEMORY_EMULATOR 65535

char line[MAX_LINE_LENGTH];
bool isMnemonic = false;
bool isLiteral = false;
int linenum = 1;
int number;
int len;
char *token;
char *mnemonic;
char *operand;

int mnemonic_index = 0;

char dest[50];
char *endptr;

bool isZeroPage = false;
bool isAbsolute = false;
bool isIndirect = false;
bool indirectX = false;
bool indirectY = false;
bool isZeroPageX = false;
bool isAbsoluteX = false;
bool isAbsoluteY = false;
int code_index = 0;

unsigned char *memory;
unsigned char *code_address;


#define MNEMONICS_SIZE 	56
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
	
	// Comparators - Different Logic for X and Y
	"CMP",
	"CPX",		// Lógica diferente nos cálculos
	"CPY",		// Lógica diferente nos cálculos
	
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
	
	// Load Instructions - Differente Logic for X and Y
	"LDA",
	"LDX",	// Lógica diferente nos cálculos
	"LDY",	// Lógica diferente nos cálculos
	
	"LSR",
	"NOP",
	"ORA",
	"ROL", 	// Lógica diferente (na sintaxe)
	"ROR",	// Lógica diferente (na sintaxe)
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
	"INY"
};
// OTHERS PRE-PROCESSING COMMANDS: DCB, DEFINE

const char* opcodes[] = {
	0x65, 0x25, 0x06, 0x24, 0x10, 0x30, 0x50, 0x70, 0x90, 0xB0, 0xD0, 0xF0,
	0x00, 0xC5, 0xE4, 0xC4, 0xC6, 0x45, 0x18, 0x38, 0x58, 0x78, 0xB8, 0xD8,
	0xF8, 0xE6, 0x4C, 0x20, 0xA5, 0xA6, 0xA4, 0x46, 0xEA, 0x05, 0x26, 0x66,
	0x40, 0x60, 0xE5, 0x85, 0x86, 0x84, 0x48, 0x68, 0x08, 0x28, 0x9A, 0xBA,
	0xAA, 0x8A, 0xCA, 0xE8, 0xA8, 0x98, 0x88, 0xC8
};

void printerr(const char* msg){
	printf("Error: Syntax error at line %d - %s.\n", linenum, msg);
}

void format_line(){
	for(int i = 0; i < strlen(line); i++){
		if(line[i] == 0x09){
			line[i] = 0x20;
		}
	}
}

void format_operand(){
	strcat(operand, token);
    token = strtok(NULL, " ");
}

void assembler(const char *filename) {
	memory = (unsigned char *) malloc(MEMORY_EMULATOR * sizeof(unsigned char));
	if (memory == NULL) {
        printf("Erro ao alocar memória.\n");
        return;
    }
    
    // Definir o endereço base de código como 0x600
    code_address = memory + 0x600;
    
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        
		bool isValid = tokenizer();
        if(!isValid)
        	break;
		
		isValid = parser();
		if(!isValid)
        	break;
        
		// Gerador...
		generator();
			
        linenum++;
    }

	unsigned short address = 0x600;
	printf("Code Length: %d\n", code_index);
	for(int i = 0; i < code_index; i++){
		if(i % 16 == 0)
			printf("\n0x%x: ", address);
		printf("%x ", code_address[i]);
		address++;
	}
		
		
    fclose(file);
}

void generator(){
    char opcode = opcodes[mnemonic_index]; // e.g. ADC = $65
    char operand_byte1, operand_byte2;
    
    if(isLiteral){
    	switch(mnemonic_index){
    		case 14:
    		case 15:
    		case 29:
    		case 30:	opcode -= 4;	// e.g. LDY = $A4 - 4 = $A0
    					break;
    		default:	opcode += 4;	// e.g. ADC = $65 + 4 = $69
		}	
    	operand_byte1 = (char) number & 0xFF;
	}else{
		if(isIndirect){
			if(indirectX) opcode -= 4;
			if(indirectY) opcode += 12;
		}
		
	    if(isZeroPage){
	    	if(isZeroPageX)
	    		opcode += 16;
	    	operand_byte1 = (char) number & 0xFF;
		}else{
			if(isAbsolute){
				opcode += 8;  	// e.g. ADC = $65 + 8 = $6D
				if(isAbsoluteX)	opcode += 16;
				if(isAbsoluteY) opcode += 12;
	    		operand_byte1 = (char) number & 0xFF;
	    		operand_byte2 = (char) ((number & 0xFF00) >> 8);
			}
		}
	}
    
    code_address[code_index++] = opcode;
    code_address[code_index++] = operand_byte1;
    if(isAbsolute)
    	code_address[code_index++] = operand_byte2;

}

bool parser(){
	//printf("Mnemonic found: , Current Mnemonic: \n");
	isZeroPage = false;
	isAbsolute = false;
	if(!isMnemonic){
		if(isLiteral){
			if(!parsing_operand(2))
        		return false;
        			
			if(len > 2){
				printerr("Exceeded the 8-bit limit for literal");
				return false;
			}
				
		}else{
			if(!parsing_operand(1))
        		return false;
        		
			if(len <= 2){
				// Zero page
				isZeroPage = true;
			}else if(len < 5){
				// Absolute
				isAbsolute = true;
			}else{
				printerr("Exceeded the 16-bit limit for address");
				return false;
			}
		}
	}else{
		// Instrução de 0 operando
		printf("No operands: %s\n", mnemonics[mnemonic_index]);
	}
	return true;
}

bool parse_addressing(int index){
	//if(isIndirect){
		char op[50] = {0};
		bool isParenthesisValid = false;
		indirectX = false;
		indirectY = false;
		int operand_len = strlen(operand);
		int i = (operand[0] == '(') ? index + 1 : index;
		int count = 0;
		
		for(; i < operand_len; i++){
			if(operand[i] != ',' && operand[i] != ')'){
				op[i-index] = operand[i];
				count++;
			}else{
				if(operand[i] == ')' && isIndirect)
					isParenthesisValid = true;
				while(++i != operand_len){
					if(operand[i] != ','){
						if(operand[i-1] != ','){
							printerr("Missing comma separator");
							return false;
						}
						indirectX = (operand[i] == 'X' || operand[i] == 'x') && count <= 2 && !isParenthesisValid && isIndirect;
						indirectY = (operand[i] == 'Y' || operand[i] == 'y') && count <= 2 && isParenthesisValid && isIndirect;
						isZeroPageX = (operand[i] == 'X' || operand[i] == 'x') && count <= 2 && !isIndirect;
						isAbsoluteX = (operand[i] == 'X' || operand[i] == 'x') && count > 2 && count < 5 && !isIndirect;
						isAbsoluteY = (operand[i] == 'Y' || operand[i] == 'y') && count > 2 && count < 5 && !isIndirect;
						//bool noBranchOrJump = mnemonic_index < 4 && mnemonic_index > 11 && mnemonic_index < 26 && mnemonic_index > 27;
						if(!indirectY && !indirectX && isIndirect){	// && verificar se é diferente de branchs e jumps
							printerr("Invalid indirect addressing");
							return false;
						}
						break;
					}
					if(operand[i] == ')' && isIndirect)
						isParenthesisValid = true;
				}
				if(!isIndirect && (!isZeroPageX && !isAbsoluteX && !isAbsoluteY)){
					printerr("Invalid addressing - Missing register");
					return false;
				}
				bool noBranchOrJump = (mnemonic_index < 4 || mnemonic_index > 11) && (mnemonic_index < 26 || mnemonic_index > 27);
				if(!indirectY && !indirectX && isIndirect && noBranchOrJump){	// && verificar se é diferente de branchs e jumps
					printerr("Invalid indirect addressing");
					return false;
				}
			}
		}
		if(!isParenthesisValid && isIndirect){
			printerr("Missing parenthesis closed");
			return false;
		}
	
	if(isIndirect)
		memcpy(dest, &op[index], count+1);
	else
		memcpy(dest, op, count+1);
	return true;	
}

bool parsing_operand(int index){
	if(!parse_addressing(index)) return false;
	number = strtol(dest, &endptr, 16);
	len = strlen(dest);
	//printf("String: %s, Hexa: %x, Decimal: %d\n", dest, number, number);
	if (*endptr != '\0') {
		printerr("Cannot parse the hexa number");
		return false;
	}
	
	return true;	
}

bool tokenizer(){
	format_line();
	token = strtok(line, " ");
	int i = 0;
	int count_tok = 0;
	
    while (token != NULL) {
    	if(count_tok >= 2){
    		format_operand();
        	count_tok++;
    		continue;
		}
        if(token[0] == '#'){
        	if(token[1] == '$'){
        		if(!setstate(true, false)) return false;
			}else{
				printerr("Operand should be a hexa number");
				return false;
			}
		}else if(token[0] == '$'){
				if(!setstate(false, false)) return false;
		}else if(token[0] == '('){
				if(!setstate(false, true)) return false;
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
			if(i == MNEMONICS_SIZE){
				printerr("Unknown mnemonic");
				return false;
			}
				
		}
        	
        token = strtok(NULL, " ");
        count_tok++;
    }
    
    mnemonic_index = i;
	return true;	
}

bool setstate(bool literal, bool indirect){
	if(!isMnemonic){
        printerr("Invalid mnemonic");
        return false;
	}
    isMnemonic = false;
    isLiteral = literal;
    isIndirect = indirect;
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
