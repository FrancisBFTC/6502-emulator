// 1st list node
struct node_def {
	int line;
	char name[256];
	char value[256];
	char refs[256];
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

// 3th list node
struct node_refs {
	int addr;
	bool relative;
	bool isDcb;
	bool isHigh;
	struct node_refs * next;
};
typedef struct node_refs RefsAddr;

// 4th list node
struct node_lab {
	int line;
	int addr;
	char name[256];
	RefsAddr * refs;
	struct node_lab * next;
};
typedef struct node_lab LabelList;


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

// Initialize the list
LabelList* begin_lab(){
	return NULL;
}

// Insert a new node
DefineList* insertdef(DefineList* list, int line, char name[], char value[], char refs[]){
	DefineList *new_node = (DefineList*) malloc(sizeof(DefineList));
	strcpy(new_node->name, name);
	if(refs == NULL){
		strcpy(new_node->value, value);
		new_node->refs[0] = refs;
	}else{
		strcpy(new_node->refs, refs);
		new_node->value[0] = value;
	}
	
	new_node->line = line;
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

// Insert a new node
LabelList* insertlab(LabelList* list, int line, char name[], int addr){
	LabelList *new_node = (LabelList*) malloc(sizeof(LabelList));
	strcpy(new_node->name, name);
	new_node->line = line;
	new_node->addr = addr;
	new_node->next = list;
	return new_node;
}

// Insert a new node
RefsAddr* insertaddr(RefsAddr* list, int addr, bool relative, bool isdcb, bool isHigh){
	RefsAddr *new_node = (RefsAddr*) malloc(sizeof(RefsAddr));
	new_node->addr = addr;
	new_node->relative = relative;
	new_node->isDcb = isdcb;
	new_node->isHigh = isHigh;
	new_node->next = list;
	return new_node;
}

// search the node
DefineList* search(DefineList *list, char* name){
	for(DefineList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li;
			
	return NULL;
}

// get the value
DefineList* getdef(DefineList *list, char* name){
	for(DefineList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li;
			
	return NULL;
}

// get the value
DcbList* getdcb(DcbList *list, int line){
	for(DcbList *li = list; li != NULL; li = li->next)
		if(li->line == line)
			return li;
			
	return NULL;
}

// get the value
LabelList* getLabelByLine(LabelList *list, int line){
	for(LabelList *li = list; li != NULL; li = li->next)
		if(li->line == line)
			return li;
			
	return NULL;
}

// get the value
LabelList* getLabelByName(LabelList *list, char name[]){
	for(LabelList *li = list; li != NULL; li = li->next)
		if(strcmp(li->name, name) == 0)
			return li;
			
	return NULL;
}

// get the value
void setref(RefsAddr *list, char *code_addr, int addr){
	for(RefsAddr *li = list; li != NULL; li = li->next){
		int op_index = li->addr + 1;
		if(li->relative){
			int PC = 0x600 + op_index - 1;
			code_addr[op_index] = ((addr & 0xFF) - (PC + 2));
		}else{
			if(li->isDcb){
				code_addr[li->addr] = (li->isHigh) ? (addr & 0xFF00) >> 8 : (addr & 0xFF);	
			}else{
				code_addr[op_index] = (addr & 0xFF);
				code_addr[op_index+1] = (addr & 0xFF00) >> 8;
			}
		}
	}
}

// show each node the list
void showdef(DefineList *list){
	for(DefineList *li = list; li != NULL; li = li->next){
		if(li->refs[0] == 0)
			printf("name = %s, value = %s, refs = none\n", li->name, li->value);
		else if(li->value[0] == 0)
				printf("name = %s, value = none, refs = %s\n", li->name, li->refs);
	}
		
		
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

// show each node the list
void showlab(LabelList *list){
	for(LabelList *li = list; li != NULL; li = li->next)
		printf("name = %s, addr = 0x%X, line = %d\n", li->name, li->addr, li->line);
}

// show each node the list
void showrefs(RefsAddr *list){
	for(RefsAddr *li = list; li != NULL; li = li->next)
		printf("addr = 0x%X, \n", li->addr);
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

// free the define list
void freelab(LabelList *list){
	LabelList *aux = list;
	
	while(aux != NULL){
		LabelList *next_node = aux->next;
		free(aux);
		aux = next_node;
	}
}

// free the define list
void freeref(RefsAddr *list){
	RefsAddr *aux = list;
	
	while(aux != NULL){
		RefsAddr *next_node = aux->next;
		free(aux);
		aux = next_node;
	}
}
