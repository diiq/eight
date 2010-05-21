#ifndef EIGHT_SYMBOLS_
#define EIGHT_SYMBOLS_

typedef struct element_struct element;

struct element_struct {
     wchar_t *name;
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
}

void insert_symbol(wchar_t *name, int val)
{
    wchar_t *nname = (wchar_t *)malloc(sizeof(wchar_t)*(wcslen(name)+1));
     wcscpy(nname, name);

     element *next = (element *)malloc(sizeof(element));
     symbol_table_tail->next = next;
     symbol_table_tail = next;
     symbol_table_tail->next = NULL;
     symbol_table_tail->name = nname;
     symbol_table_tail->id = val;
}


symbol_id string_to_symbol_id(wchar_t *name)
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

symbol_id recursive_string_to_symbol_id(wchar_t* name, element *root)
{
  
     if (wcscmp(name, root->name) == 0){
	  return root->id;
     } else if(root->next == NULL){
	  return -1;
     } else {
	  return recursive_string_to_symbol_id(name, root->next);
     }
}


wchar_t* recursive_symbol_id_to_string(symbol_id id, element *root)
{
     if (id == root->id){
	  return root->name;
     } else if(root->next == NULL){
	  return NULL;
     } else {
	  return recursive_symbol_id_to_string(id, root->next);
     }
}

wchar_t* symbol_id_to_string(symbol_id sym){
     return recursive_symbol_id_to_string(sym, symbol_table_root);
}

closure* string_to_symbol(closure *a){
    // Watch it! Takes an eight string, not a c-string.
    if (!stringp(a))
	error(121, 121, "Oh man, I tried to make a symbol out of a non-string.");
    int l = length(a);
    wchar_t name[50] = {L'\0'};
    int i = 0;
    for(i=0; i<l; i++){
	name[i] = car(a)->in->character;
	a = cdr(a);
    }
    return symbol(string_to_symbol_id(name));
}

#endif
