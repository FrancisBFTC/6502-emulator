// 1st list node
struct node_def {
	char name[256];
	char value[256];
	struct node_def * next;
};
typedef struct node_def DefineList;

// 2nd list node
struct node_dcb {
	int line;
	int length;
	unsigned char* value;
	struct node_dcb * next;
};
typedef struct node_dcb DcbList;

// DAT/TAD: Data Abstract Type Begin
// -----------------------------------------------------------------
// Initialize the list
DefineList* begin_def(){
	return NULL;
}

// Initialize the list
DcbList* begin_dcb(){
	return NULL;
}

// Insert a new node
DefineList* insertdef(DefineList* list, char name[], char value[]){
	DefineList *new_node = (DefineList*) malloc(sizeof(DefineList));
	strcpy(new_node->name, name);
	strcpy(new_node->value, value);
	new_node->next = list;
	return new_node;
}

// Insert a new node
DcbList* insertdcb(DcbList* list, int line, int length, char* value){
	DcbList *new_node = (DcbList*) malloc(sizeof(DcbList));
	new_node->line = line;
	new_node->length = length;
	new_node->value = (char*) malloc(length);
	memcpy(new_node->value, value, length);
	new_node->next = list;
	return new_node;
}

// Remove a node at begin or mid the list
/*
DefineList* remove(DefineList *list, int id){
	Node *previous = NULL;			// Pointer to previous node
	Node *aux = list;				// Pointer to list
	
	// search the node and store the previous node
	while(aux != NULL && aux->id != id){
		previous = aux;
		aux = aux->next;
	}
	
	if(aux == NULL) return list; 	// if not found, return the original
									// but if found ...
									
	if(previous == NULL)			// begin of list point to the next
		list = aux->next;			// remove on the begin of list
	else							// previous point to the next
		previous->next = aux->next;	// remove on the mid of list
	
	free(aux); 			// Free memory space (remove the node)
	return list;
}
*/

// search the node
DefineList* search(DefineList *list, char* name){
	for(DefineList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li;
			
	return NULL;
}

// get the value
char* getdef(DefineList *list, char* name){
	for(DefineList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li->value;
			
	return NULL;
}

// get the value
DcbList* getdcb(DcbList *list, int line){
	for(DcbList *li = list; li != NULL; li = li->next)
		if(li->line == line)
			return li;
			
	return NULL;
}

// show each node the list
void showdef(DefineList *list){
	for(DefineList *li = list; li != NULL; li = li->next)
		printf("name = %s, value = %s\n", li->name, li->value);
}

// show each node the list
void showdcb(DcbList *list){
	for(DcbList *li = list; li != NULL; li = li->next){
		printf("line = %d, length = %d,", li->line, li->length);
		printf(" values = ");
		for(int i = 0; i < li->length; i++)
			printf("%d,", li->value[i]);
		printf("\n");
	}
}

// free the define list
void freedef(DefineList *list){
	DefineList *aux = list;
	
	while(aux != NULL){
		DefineList *next_node = aux->next;
		free(aux);
		aux = next_node;
	}
}

// free the dcb list
void freedcb(DcbList *list){
	DcbList *aux = list;
	
	while(aux != NULL){
		DcbList *next_node = aux->next;
		free(aux->value);
		free(aux);
		aux = next_node;
	}
}
