#ifndef EIGHT_SYMBOLS_
#define EIGHT_SYMBOLS_

typedef struct element_struct element;

struct element_struct {
     char* name;
     symbol_id id;
     element *next;
}; 

element *symbol_table_root;
element *symbol_table_tail;
int current = 250;

void initialize_symbol_table()
{
     symbol_table_root = (element *)malloc(sizeof(element));
     symbol_table_tail = symbol_table_root;
     symbol_table_root->id = -1;			
     symbol_table_root->name = NULL;
     insert_symbol("...", ELIPSIS);
     insert_symbol("asterix", ASTERIX);
     insert_symbol("comma", COMMA);
     insert_symbol("quote", QUOTE);
     insert_symbol("atpend", ATPEND);
     insert_symbol("leaked", LEAKED);
     insert_symbol("t", T);
}

void insert_symbol(char *name, int val)
{
     char *nname = (char *)malloc(sizeof(char)*strlen(name));
     strcpy(nname, name);

     element *next = (element *)malloc(sizeof(element));
     symbol_table_tail->next = next;
     symbol_table_tail = next;
     symbol_table_tail->next = NULL;
     symbol_table_tail->name = nname;
     symbol_table_tail->id = val;
}


symbol_id string_to_symbol_id(char *name)
{
     symbol_id ret = recursive_string_to_symbol_id(name, 
						   symbol_table_root->next);    
     if (ret != -1) {
	  return ret;
     } else {
	  symbol_id id = current++;
	  insert_symbol(name, id);
	  return id;
     }
}

symbol_id recursive_string_to_symbol_id(char* name, element *root)
{
  
     if (strcmp(name, root->name) == 0){
	  return root->id;
     } else if(root->next == NULL){
	  return -1;
     } else {
	  return recursive_string_to_symbol_id(name, root->next);
     }
}


char* recursive_symbol_id_to_string(symbol_id id, element *root)
{
     if (id == root->id){
	  return root->name;
     } else if(root->next == NULL){
	  return NULL;
     } else {
	  return recursive_symbol_id_to_string(id, root->next);
     }
}

char* symbol_id_to_string(symbol_id sym){
     return recursive_symbol_id_to_string(sym, symbol_table_root);
}

#endif
