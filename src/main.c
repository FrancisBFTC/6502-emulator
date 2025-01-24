#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

void processFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, " ");
        while (token != NULL) {
        	printf("Token: %s ", token);
        	char dest[50];
        	if(token[0] == '#' && token[1] == '$'){
        		strcpy(dest, &token[2]);
        		int number = atoi(dest);
        		printf(" -> 0x%d (Literal)", number);
			}else if(token[0] == '$'){
				strcpy(dest, &token[1]);
        		int address = atoi(dest);
        		printf(" -> 0x%d (Endereco)", address);
			}else{
				printf(" -> mnemonico");
			}
			printf("\n");
            token = strtok(NULL, " ");
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    processFile(argv[1]);

    return EXIT_SUCCESS;
}
