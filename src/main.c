#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool tokenizer(void);
bool parser(void);
bool parsing_numbers(int);
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

	for(int i = 0; i < code_index; i++)
		printf("$%x ", code_address[i]);
		
    fclose(file);
}

void generator(){
    char opcode = opcodes[mnemonic_index]; // e.g. ADC = $65
    char operand_byte1, operand_byte2;
    
    if(isLiteral){
    	opcode += 4;	// e.g. ADC = $65 + 4 = $69
    	operand_byte1 = (char) number & 0xFF;
	}else{
		if(indirectX) opcode -= 4;
		if(indirectY) opcode += 12;
	    if(isZeroPage){
	    	operand_byte1 = (char) number & 0xFF;
		}else{
			if(isAbsolute){
				opcode += 8;  	// e.g. ADC = $65 + 8 = $6D
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
	printf("Mnemonic found: %s, Current Mnemonic: %s\n", mnemonics[mnemonic_index], mnemonic);
	isZeroPage = false;
	isAbsolute = false;
	if(!isMnemonic){
		if(isLiteral){
			if(!parsing_numbers(2))
        		return false;
        			
			if(len > 2){
				printerr("Exceeded the 8-bit limit for literal");
				return false;
			}
				
		}else{
			if(!parsing_numbers(1))
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

bool parsing_numbers(int index){
	if(isIndirect){
		bool isParenthesisValid = false;
		indirectX = false;
		indirectY = false;
		char op[50];
		//dest = (char*) malloc(strlen(operand));
		int operand_len = strlen(operand);
		int i = index;
		for(; i < operand_len; i++){
			if(operand[i] != ',' && operand[i] != ')'){
				op[i-index] = operand[i];
			}else{
				if(operand[i] == ')')
					isParenthesisValid = true;
				while(++i != operand_len){
					if(operand[i] != ' ' && operand[i] != ','){
						indirectX = (operand[i] == 'X' || operand[i] == 'x') && !isParenthesisValid;
						indirectY = (operand[i] == 'Y' || operand[i] == 'y') && isParenthesisValid;
						if(!indirectY && !indirectX){
							printerr("Invalid indirect addressing");
							return false;
						}
							
						break;
					}
					if(operand[i] == ')')
						isParenthesisValid = true;
				}
			}
		}
		if(!isParenthesisValid){
			printerr("Missing parenthesis closed");
			return false;
		}
		memcpy(dest, &op[index], 50);
	}else{
		strcpy(dest, &operand[index]);
	}
	
	//dest = &op[index];
	number = strtol(dest, &endptr, 16);
	len = strlen(dest);
	printf("String: %s, Hexa: %x, Decimal: %d\n", dest, number, number);
	if (*endptr != '\0') {
		printerr("Cannot parse the hexa number");
		return false;
	}
	
	//free(dest);
	return true;	
}

bool tokenizer(){
	token = strtok(line, " ");
	int i = 0;
	
    while (token != NULL) {
        if(token[0] == '#'){
        	if(token[1] == '$'){
        		if(!isMnemonic){
        			printerr("Invalid mnemonic");
        			return false;
				}
        		operand = token;
        		isMnemonic = false;
        		isLiteral = true;
        		isIndirect = false;
			}else{
				printerr("Operand should be a hexa number");
				return false;
			}
		}else if(token[0] == '$'){
				if(!isMnemonic){
        			printerr("Invalid mnemonic");
        			return false;
				}
        		operand = token;
        		isMnemonic = false;
				isLiteral = false;
				isIndirect = false;
		}else if(token[0] == '('){
				if(!isMnemonic){
        			printerr("Invalid mnemonic");
        			return false;
				}
				operand = token;
				isMnemonic = false;
				isLiteral = false;
				isIndirect = true;
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

int main(int argc, char *argv[]) {
	if (argc == 1) {
        fprintf(stderr, "Usage: %s -m |--mount <arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

	if(strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--mount") == 0)
    	assembler(argv[2]);

    return EXIT_SUCCESS;
}
