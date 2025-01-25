#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool tokenizer(void);
bool parser(void);
bool parsing_numbers(int);

#define MAX_LINE_LENGTH 1024
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

#define MNEMONICS_SIZE 	4
const char* mnemonics[] = {
	"LDA",
	"STA",
	"INX"
};

void printerr(const char* msg){
	printf("Error: Syntax error at line %d - %s.\n", linenum, msg);
}

void assembler(const char *filename) {
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
			
        linenum++;
    }

    fclose(file);
}

bool parser(){
	printf("Mnemonic found: %s, Current Mnemonic: %s\n", mnemonics[mnemonic_index], mnemonic);
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
			}else if(len < 5){
				// Absolute
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
	strcpy(dest, &operand[index]);
	number = strtol(dest, &endptr, 16);
	len = strlen(dest);
	printf("String: %s, Hexa: %x, Decimal: %d\n", dest, number, number);
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
        		if(!isMnemonic){
        			printerr("Invalid mnemonic");
        			return false;
				}
        		operand = token;
        		isMnemonic = false;
        		isLiteral = true;
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
