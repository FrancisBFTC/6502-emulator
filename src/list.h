// 1st list node
struct node_def {
	char name[256];
	char value[256];
	struct node_def * next;
};
typedef struct node_def DefineList;

// DAT/TAD: Data Abstract Type Begin
// -----------------------------------------------------------------
// Initialize the list
DefineList* begin(){
	return NULL;
}

// Insert a new node
DefineList* insert(DefineList* list, char name[], char value[]){
	DefineList *new_node = (DefineList*) malloc(sizeof(DefineList));
	strcpy(new_node->name, name);
	strcpy(new_node->value, value);
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
char* get(DefineList *list, char* name){
	for(DefineList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li->value;
			
	return NULL;
}

// show each node the list
void show(DefineList *list){
	for(DefineList *li = list; li != NULL; li = li->next)
		printf("name = %s, value = %s\n", li->name, li->value);
}

// free the list
void freel(DefineList *list){
	DefineList *aux = list;
	
	while(aux != NULL){
		DefineList *next_node = aux->next;
		free(aux);
		aux = next_node;
	}
}
