/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley (diiq, stm31415@gmail.com)

***************************************************************************/
#ifndef EIGHT_SYMBOLS_
#define EIGHT_SYMBOLS_

#include "hashtable.h"
#include "hashtable.c"
#include "hashtable_itr.h"
#include "hashtable_itr.c"
#include "eight.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

int current_symbol_value;
void *Symbol_table;

static unsigned int      symbol_hash_fn(void *k);
static int               symbol_key_equality (void *key1, void *key2);


void initialize_symbol_table()
{
     Symbol_table=create_hashtable(16, symbol_hash_fn, symbol_key_equality);
     current_symbol_value = MAX_SPECIAL;
}

void insert_symbol(char *name, int val)
{
     int *vala = GC_MALLOC_ATOMIC(sizeof(int));
     *vala = val;
     char *key = GC_MALLOC_ATOMIC(sizeof(char)*(strlen(name)+1));
     key = strcpy(key, name);
     if (!hashtable_insert(Symbol_table, key, vala)){
          printf("Symbol error!");
     }
}

symbol_id string_to_symbol_id(char *name)
{
     int *ret;
     if (NULL == (ret = (int *)hashtable_search(Symbol_table, name))) {
          int car = current_symbol_value++;
          insert_symbol(name, car);
          return car;
     }
     return *ret;
}

char* symbol_id_to_string(symbol_id sym) 
{
     int *v;
     char *k;
     if (hashtable_count(Symbol_table) > 0)
     {
	  struct hashtable_itr *itr = hashtable_iterator(Symbol_table);
	  do {
	       k = hashtable_iterator_key(itr);
	       v = hashtable_iterator_value(itr);
	       if (*v == sym){
		    return k;
	       }
	  } while (hashtable_iterator_advance(itr));
	  free(itr);
     }
     return NULL;
}

static unsigned int symbol_hash_fn(void *str)
{
     unsigned int hash = 5381;
     int c;
     while ((c = *(char*)str++))
          hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
     return (unsigned int)hash;
}

static int symbol_key_equality(void *stra, void *strb)
{
     if(strcmp((char*)stra, (char*)strb)==0){
          return 1;
     } else {
          return 0;
     }
}


#endif
