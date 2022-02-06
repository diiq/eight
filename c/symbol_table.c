/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley


A hash-table that expands logarithmically & chains

***************************************************************************/
#ifndef EIGHT_SYMBOL_TABLE
#define EIGHT_SYMBOL_TABLE

#include "eight.h"

#define start_table_iteration(table, item, i)	\
    for(i = 0; i<table->size; i++){		\
        chain = table->array[i];		\
        if(chain != NULL){		      	\
            while(!nilp(chain)){       		\
	    item = cheap_car(chain);		\
                do

#define end_table_iteration while(0);		\
    chain = cheap_cdr(chain);			\
            }	       				\
        }	                                \
    }



void table_intersection(symbol_table *a, 
		  symbol_table *b, 
		  symbol_table *intersection, 
		  symbol_table *a_difference,
		  symbol_table *b_difference);

void resize_table(symbol_table *table);

int symbol_hash(closure *symbol, symbol_table *table)
{
    return symbol->in->symbol_id % table->size;
}

symbol_table *new_symbol_table()
{
    symbol_table *ret = new(symbol_table);
    ret->size = INITIAL_TABLE_SIZE;
    ret->entries = 0;
    //    ret->array = calloc(INITIAL_TABLE_SIZE, sizeof(closure *));
    ret->type = SYMBOL_TABLE;
    return ret;
}

void table_insert(closure *symbol, closure *value, symbol_table *table)
{
    int hash = symbol_hash(symbol, table);
    closure *chain = table->array[hash];
    if (chain == NULL)
	chain = nil();
    closure *prev = assoc(symbol, chain);
    if (!nilp(prev)){
	prev->in->cons->car = value;
    } else {
	table->array[hash] = cheap_cons(cheap_list(2, symbol, value), chain);
    }
    table->entries++;
}

closure *table_lookup(closure *symbol, symbol_table *table)
{
    // TODO resize array here when load too great.
    int hash = symbol_hash(symbol, table);
    closure *chain = table->array[hash];
    if (chain == NULL)
	return nil();
    
    return assoc(symbol, chain);
}

symbol_table *table_union(symbol_table *a, symbol_table *b)
{
    symbol_table *ret = new_symbol_table();
    closure *item;
    int i;
    closure *chain;			            
    start_table_iteration(b, item, i){
	if (!leakedp(cheap_cdr(item)) ||
	    nilp(table_lookup(cheap_car(item), ret)))
	    table_insert(cheap_car(item), second(item), ret);
    } end_table_iteration;

    start_table_iteration(a, item, i){
	if (!leakedp(cheap_cdr(item)) ||
	    nilp(table_lookup(cheap_car(item), ret)))
	table_insert(cheap_car(item), second(item), ret);
    } end_table_iteration;
    
    return ret;
}

symbol_table *closing_to_table(closure *a)
{
    symbol_table *ret = new_symbol_table();
    while(!nilp(a)){
	if(nilp(table_lookup(cheap_car(cheap_car(a)), ret))){
	    table_insert(cheap_car(cheap_car(a)), 
			 second(cheap_car(a)),
			 ret);
	}
	a = cheap_cdr(a);
    }
    return ret;
}

closure *table_to_assoc(symbol_table *a)
{
    closure *item;
    int i;
    closure *chain;			            
    closure *ret = nil();
    for(i = 0; i<a->size; i++){		    
	if(a->array[i] != NULL){
	    ret = append(ret, a->array[i]); 
	}
    }
    return ret;
}


#endif
