#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <windows.h>

#define MAX_LINE_LENGTH 1024

char line[MAX_LINE_LENGTH];
char* token;
char* success = NULL;
char* error = NULL;
char* testname = NULL;
char* input = NULL;
char* output = NULL;
char* setup = NULL;
char* success_test = NULL;
char* error_test = NULL;
char* setup_test = NULL;
char* name = NULL;

int linenum = 1;

bool isMessage = false;
bool isTest = false;
bool isConcatInput = false;
bool isConcatOutput = false;



void format_line(){
	line[strcspn(line, "\n")] = '\0';
	bool block = false;
	for(int i = 0; i < strlen(line); i++){
		if(block && line[i] == '\"'){
			block = false;
			continue;
		}
			
		if(!block){
			if(line[i] == 0x09)
				line[i] = 0x20;
			else if(line[i] > 0x60 && line[i] < 0x7B)
					line[i] -= 0x20;
		}
		
		block = (line[i] == '\"') || block;
	}
}

void print_msg_error(){
	if(isMessage)
		printf("Error at line %d - Missing '='", linenum);
	else
		printf("Error at line %d - It's not in a message block", linenum);	
}

void print_test_error(){
	if(isTest)
		printf("Error at line %d - Missing '='", linenum);
	else
		printf("Error at line %d - It's not in a test block", linenum);	
}

void save_file(const char* filename, const char* data){
	FILE *file = fopen(filename, "w");
	
	if(file == NULL){
		printf("Error in open the file!");
		return;
	}
	
	fprintf(file, "%s", data);
	fclose(file);
}

char* read_file(const char* filename){
	FILE *file = fopen(filename, "r");
	
	if(file == NULL){
		printf("Error in open the file!");
		return;
	}
	
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* datas = (char*) malloc(size + 1);
	if(datas == NULL){
		printf("Error in allocate memory!");
		fclose(file);
		return NULL;
	}
	
	fread(datas, 1, size, file);
	datas[size] = '\0';
	fclose(file);
	
	return datas;
}

/*
char* testname = NULL;
char* input = NULL;
char* output = NULL;
char* setup = NULL;
char* success_test = NULL;
char* error_test = NULL;
char* setup_test = NULL;
char* name = NULL;
*/
void reset_all(){
	if(name != NULL)
		free(name);
	if(success != NULL)
		free(success);
	if(error != NULL)
		free(error);
	if(setup != NULL)
		free(setup);
	if(testname != NULL)
		free(testname);
	if(input != NULL)
		free(input);
	if(output != NULL)
		free(output);
	if(success_test != NULL)
		free(success_test);
	if(error_test != NULL)
		free(error_test);
	if(setup_test != NULL)
		free(setup_test);
				
	success = NULL;
	error = NULL;
	setup = NULL;
	name = NULL;
	testname = NULL;
	input = NULL;
	output = NULL;
	success_test = NULL;
	error_test = NULL;
	setup_test = NULL;
}

void change_color(int textColor, int backColor){
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, (backColor << 4) | textColor);
}

bool tester(const char* filename, bool verbose, int* scount, int* fcount, int* tests){
	FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return false;
    }
    
    isMessage = false;
    isTest = false;
    bool isSuccessMsg = false;
    bool isErrorMsg = false;
    bool isNameMsg = false;
    bool isSuccessLocal = false;
    bool isErrorLocal = false;
    bool isSetupLocal = false;
    bool isInputMsg = false;
    bool isOutputMsg = false;
    bool isSetupMsg = false;
    
    int success_count = 0;
    int fail_count = 0;
    int test_count = 0;
    
    while (fgets(line, sizeof(line), file)) {
    	format_line();
    	char linetmp[MAX_LINE_LENGTH] = {0};
    	strcpy(linetmp, line);
		token = strtok(line, " ");
		int i = 0;
		int index = 0;
		int len = 0;
		bool isSuccessTmp = false;
		bool isErrorTmp = false;
		bool isNameTmp = false;
		bool isInputTmp = false;
		bool isOutputTmp = false;
		bool isSetupTmp = false;
		bool isSuccessTest = false;
		bool isErrorTest = false;
		bool isSetupTest = false;
		bool isConcat = true;
		
		while (token != NULL) {
			bool isMessageSymbol = strcmp(token, "MESSAGE{") == 0;
			bool isSuccessSymbol = strcmp(token, "SUCCESS=") == 0;
			bool isErrorSymbol = strcmp(token, "ERROR=") == 0;
			bool isInputSymbol = strcmp(token, "INPUT=") == 0;
			bool isOutputSymbol = strcmp(token, "OUTPUT=") == 0;
			bool isSetupSymbol = strcmp(token, "SETUP=") == 0;
			bool isNameSymbol = strcmp(token, "NAME=") == 0;
			
			// TODO: Corrigir error de Token Success e Error com caracteres tudo junto
			if(strcmp(token, "MESSAGE") == 0 || isMessageSymbol){
				if(isMessageSymbol || linetmp[strcspn(&linetmp[0], "{")] == '{'){
					reset_all();
					
					isSuccessMsg = false;
				    isErrorMsg = false;
				    isNameMsg = false;
				    isSuccessLocal = false;
				    isErrorLocal = false;
				    isSetupLocal = false;
				    isInputMsg = false;
				    isOutputMsg = false;
				    isSetupMsg = false;
					isMessage = true;
					break;
				}else{
					printf("Error at line %d - Missing '{'", linenum);
					return false;
				}
			}else if(strcmp(token, "TEST") == 0){
					if(linetmp[strcspn(&linetmp[0], "{")] == '{'){
						token = strtok(NULL, " ");
						token[strcspn(&token[0], "{")] = '\0';
						int tokenlen = strlen(token);
						if(testname != NULL)
							free(testname);
						testname = (char*) malloc(tokenlen+1);
						strcpy(testname, token);
						isTest = true;
						break;
					}else{
						printf("Error at line %d - Missing '{'", linenum);
						return false;
					}
					
			}else if(strcmp(token, "NAME") == 0 || isNameSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isNameSymbol || linetmp[index] == '=') && isMessage){
						isNameMsg = true;
						isNameTmp = isNameMsg;
					}else{
						print_msg_error();
						return false;
					}
			}else if(strcmp(token, "SUCCESS") == 0 || isSuccessSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isSuccessSymbol || linetmp[index] == '=') && (isMessage || isTest)){
						if(!isTest){
							isSuccessMsg = true;
							isSuccessTmp = isSuccessMsg;
						}else{
							isSuccessLocal = true;
							isSuccessTest = isSuccessLocal;
						}
					}else{
						print_msg_error();
						return false;
					}
			}else if(strcmp(token, "ERROR") == 0 || isErrorSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isErrorSymbol || linetmp[index] == '=') && (isMessage || isTest)){
						if(!isTest){
							isErrorMsg = true;
							isErrorTmp = isErrorMsg;	
						}else{
							isErrorLocal = true;
							isErrorTest = isErrorLocal;
						}
					}else{
						print_msg_error();
						return false;
					}
			}else if(strcmp(token, "INPUT") == 0 || isInputSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isInputSymbol || linetmp[index] == '=') && isTest){
						isInputMsg = true;
						isInputTmp = isInputMsg;
					}else{
						print_test_error();
						return false;
					}
			}else if(strcmp(token, "OUTPUT") == 0 || isOutputSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isOutputSymbol || linetmp[index] == '=') && isTest){
						isOutputMsg = true;
						isOutputTmp = isOutputMsg;
					}else{
						print_test_error();
						return false;
					}
			}else if(strcmp(token, "SETUP") == 0 || isSetupSymbol){
					index = strcspn(&linetmp[0], "=");
					len = strlen(&linetmp[index + 1]);
					if((isSetupSymbol || linetmp[index] == '=') && (isMessage || isTest)){
						if(!isTest){
							isSetupMsg = true;
							isSetupTmp = isSetupMsg;	
						}else{
							isSetupLocal = true;
							isSetupTest = isSetupLocal;
						}
						
					}else{
						print_test_error();
						return false;
					}
			}else{
				if(token[0] == '}'){
					if(isMessage){
						isMessage = false;
						change_color(11, 0);
    					if(isNameMsg)
    						printf("Processing '%s'...\n", name);
    					else
    						printf("Processing Tests...\n");
    					if(name != NULL){
    						free(name);
    						name = NULL;
						}
					}else if(isTest){
						
						change_color(12, 0);
						printf("\ttest ");
						change_color(6, 0);
						printf("%s", testname);
						change_color(12, 0);
						printf("\t->");
						
						save_file("input.dat", input);
						
						if(isSetupLocal){
							const char* data = " input.dat > output.dat";
							setup_test = (char*) realloc(setup_test, strlen(setup_test) + strlen(data) + 2);
							strcat(setup_test, data);
							system(setup_test);
						}else{
							system(setup);
						}
		
						char* content = read_file("output.dat");
						
						if(strcmp(content, output) == 0){
							success_count++;
							change_color(10, 0);
							if(isSuccessLocal)
								printf(" %s\n", success_test);
							else
								printf(" %s\n", success);
						}else{
							fail_count++;
							change_color(4, 0);
							if(isErrorLocal)
								printf(" %s\n", error_test);
							else
								printf(" %s\n", error);
						}
						
						if(verbose){
							change_color(14, 0);
							printf("\t\tInput Test: ");
							if(isConcatInput)
								printf("\n");
							change_color(7, 0);
							printf("%s\n", input);
							
							change_color(14, 0);
							printf("\t\tExpect Output: ");	
							if(isConcatOutput)
								printf("\n");
							change_color(7, 0);
							printf("%s\n", output);
							
							change_color(14, 0);
							printf("\t\tMessage Success: ");
							change_color(7, 0);
							if(isSuccessLocal)
								printf(" %s\n", success_test);
							else
								printf(" %s\n", success);
							
							change_color(14, 0);
							printf("\t\tMessage Error: ");
							change_color(7, 0);
							if(isErrorLocal)
								printf(" %s\n", error_test);
							else
								printf(" %s\n", error);
								
							change_color(14, 0);
							printf("\t\tSetup: ");
							change_color(7, 0);
							if(isSetupLocal)
								printf(" %s\n", setup_test);
							else
								printf(" %s\n", setup);	
						}
						
						change_color(7, 0);
						
						if(isSuccessLocal){
							free(success_test);
							success_test = NULL;
							isSuccessLocal = false;
						}
								
						if(isErrorLocal){
							free(error_test);
							error_test = NULL;
							isErrorLocal = false;
						}
						
						if(isSetupLocal){
							free(setup_test);
							setup_test = NULL;
							isSetupLocal = false;
						}
							
						isConcatInput = false;
						isConcatOutput = false;
						free(input);
						free(output);
						input = NULL;
						output = NULL;
						isTest = false;
						test_count++;
					}
					break;
				}	
			} 
			
			if(isSuccessTmp || isErrorTmp || isNameTmp || isInputTmp || isOutputTmp || isSuccessTest || isErrorTest || isSetupTmp || isSetupTest){
				
				int posQuote = 0;
				int counter = 0;
				
				while(isConcat){
					while(token != NULL){
						isConcat = token[0] == '&' && !isSetupTmp;
						token = strtok(NULL, " ");
					}
				
					char buffer[len + 1];
					i = 0;
					for(i = index + 1; i < len+index; i++){
						if(linetmp[i] == ' ')
							continue;
						break;
					}
					
					if(linetmp[i] != '"'){
						printf("Error at line %d - Missing '\"' at beginning", linenum);
						return false;
					}
					i = i + 1;
					int position = strcspn(&linetmp[i], "\"");
					posQuote += position;
					
					
					if(linetmp[i+position] != '\"'){
						printf("char: %c\n", linetmp[i+position]);
						printf("Error at line %d - Missing '\"' in the end", linenum);
						return false;
					}else{
						memcpy(buffer, &linetmp[i], position);
						buffer[position] = '\0';
						if(isSuccessTmp){
							if(success != NULL && !counter)
								free(success);
							success = (success == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(success, posQuote + 1);
							if(!counter)
								strcpy(success, buffer);
							else{
								strcat(success, "\n");
								strcat(success, buffer);
							}
							success[posQuote] = '\0';
							posQuote += 2;
						}else if(isErrorTmp){
							if(error != NULL && !counter)
								free(error);
							error = (error == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(error, posQuote + 1);
							if(!counter)
								strcpy(error, buffer);
							else{
								strcat(error, "\n");
								strcat(error, buffer);
							}
							error[posQuote] = '\0';
							posQuote += 2;
						}else if(isNameTmp){
							if(name != NULL && !counter)
								free(name);
							name = (name == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(name, posQuote + 1);
							if(!counter)
								strcpy(name, buffer);
							else{
								strcat(name, "\n");
								strcat(name, buffer);
							}
							name[posQuote] = '\0';
							posQuote += 2;
						}else if(isInputTmp){
							if(input != NULL && !counter)
								free(input);
							input = (input == NULL) ? (char*) malloc(posQuote + 1) : realloc(input, posQuote + 1);
							if(!counter)
								strcpy(input, buffer);
							else{
								strcat(input, "\n");
								strcat(input, buffer);
								isConcatInput = true;
							}
							input[posQuote] = '\0';
							posQuote += 2;
						}else if(isOutputTmp){
							if(output != NULL && !counter)
								free(output);
							output = (output == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(output, posQuote + 1);
							if(!counter)
								strcpy(output, buffer);
							else{
								strcat(output, "\n");
								strcat(output, buffer);
								isConcatOutput = true;
							}
							output[posQuote] = '\0';
							posQuote += 2;
						}else if(isSuccessTest){
							success_test = (success_test == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(success_test, posQuote + 1);
							if(!counter)
								strcpy(success_test, buffer);
							else{
								strcat(success_test, "\n");
								strcat(success_test, buffer);
							}
							success_test[posQuote] = '\0';
							posQuote += 2;
						}else if(isErrorTest){
							error_test = (error_test == NULL) ? (char*) malloc(posQuote + 1) : (char*) realloc(error_test, posQuote + 1);
							if(!counter)
								strcpy(error_test, buffer);
							else{
								strcat(error_test, "\n");
								strcat(error_test, buffer);
							}
							error_test[posQuote] = '\0';
							posQuote += 2;
						}else if(isSetupTmp){
							if(setup != NULL)
								free(setup);
							
							setup = (char*) malloc(posQuote + 1);
							strcpy(setup, buffer);
							setup[posQuote] = '\0';
							const char* data = " input.dat > output.dat";
							setup = (char*) realloc(setup, strlen(setup) + strlen(data) + 2);
							strcat(setup, data);
						}else if(isSetupTest){
							if(setup_test != NULL)
								free(setup_test);
							setup_test = (char*) malloc(posQuote + 1);
							strcpy(setup_test, buffer);
							setup_test[posQuote] = '\0';
						}
						if(isConcat){
							fgets(line, sizeof(line), file);
							format_line();
							strcpy(linetmp, line);
							token = strtok(line, " ");
							index = -1;
							len = strlen(linetmp);
						}
					}	
					counter++;
				}
				break;
				
			}else{
				printf("Error at line %d - No knownledge variable or command\n", linenum);
				return false;
			}
			
			token = strtok(NULL, " ");
		}
		
		linenum++;
	}
	
	*scount = success_count;
	*fcount = fail_count;
	*tests = test_count;
	
	reset_all();
		
	fclose(file);
	return true;
}

void show_info(int total, int passed, int fails){
	change_color(13, 0);
	printf(" %d ", total);
	change_color(11, 0);
	printf("tests runned.");
	change_color(13, 0);
	printf(" %d ", passed);
	change_color(11, 0);
	printf("passed.");
	change_color(13, 0);
	printf(" %d ", fails);
	change_color(11, 0);
	printf("failed.\n");
	change_color(7, 0);
}
int main(int argc, char *argv[]) {
	int passed = 0;
	int fails = 0;
	int total = 0;
	bool interpreted = true;
	bool isVerbose = strcmp(argv[argc-1], "-v") == 0 || strcmp(argv[argc-1], "--verbose") == 0;
	if(argv[1] == NULL){
		printf("Especifique um arquivo para testes!");
		return 1;
	}
	
	int quant = (isVerbose) ? argc - 1 : argc;
	
	for(int i = 1; i < quant; i++){
		interpreted = (isVerbose) 	? tester(argv[i], true, &passed, &fails, &total)
									: tester(argv[i], false, &passed, &fails, &total);
		if(interpreted)
			show_info(total, passed, fails);
			
		total = 0;
		fails = 0;
		passed = 0;
	}
	
	return 0;
}
