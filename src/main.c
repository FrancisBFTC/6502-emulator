#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool tokenizer(void);
bool parser(void);
bool parsing_operand(int);
bool parse_addressing(int);
bool setstate(bool, bool);
void generator(void);

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


#define MNEMONICS_SIZE 	4
const char* mnemonics[] = {
	"ADC",
	"AND",
	"ASL"
};

const char* opcodes[] = {
	0x65, 0x25, 0x06
};

void printerr(const char* msg){
	printf("Error: Syntax error at line %d - %s.\n", linenum, msg);
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
    	opcode += 4;	// e.g. ADC = $65 + 4 = $69
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
		int i = index;
		int count = 0;
		
		for(; i < operand_len; i++){
			if(operand[i] != ',' && operand[i] != ')'){
				op[i-index] = operand[i];
				count++;
			}else{
				if(operand[i] == ')' && isIndirect)
					isParenthesisValid = true;
				while(++i != operand_len){
					if(operand[i] != ' ' && operand[i] != ','){
						indirectX = (operand[i] == 'X' || operand[i] == 'x') && !isParenthesisValid && isIndirect;
						indirectY = (operand[i] == 'Y' || operand[i] == 'y') && isParenthesisValid && isIndirect;
						isZeroPageX = (operand[i] == 'X' || operand[i] == 'x') && count <= 2 && !isIndirect;
						isAbsoluteX = (operand[i] == 'X' || operand[i] == 'x') && count > 2 && count < 5 && !isIndirect;
						isAbsoluteY = (operand[i] == 'Y' || operand[i] == 'y') && count > 2 && count < 5 && !isIndirect;
						if(!indirectY && !indirectX && isIndirect){
							printerr("Invalid indirect addressing");
							return false;
						}
						break;
					}
					if(operand[i] == ')' && isIndirect)
						isParenthesisValid = true;
				}
			}
		}
		if(!isParenthesisValid && isIndirect){
			printerr("Missing parenthesis closed");
			return false;
		}
	
	if(isIndirect)
		memcpy(dest, &op[index], 50);
	else
		memcpy(dest, op, 50);
	//}else{
		//strcpy(dest, &operand[index]);
	//}
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
	token = strtok(line, " ");
	int i = 0;
	
    while (token != NULL) {
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
