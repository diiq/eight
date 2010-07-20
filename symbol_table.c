/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley (diiq, stm31415@gmail.com)


A hash-table that expands logarithmically & chains

***************************************************************************/
#ifndef EIGHT_SYMBOL_TABLE
#define EIGHT_SYMBOL_TABLE

#include "eight.h"

#define INITIAL_TABLE_SIZE 50


#define start_table_iteration(table, item, i)	\
    for(i = 0; i<table->size; i++){		\
        chain = table->array[i];		\
        if(chain != NULL){		      	\
            while(!nilp(chain)){       		\
                item = car(chain);	        \
                do

#define end_table_iteration while(0);		\
    chain = cdr(chain);				\
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
    ret->type = TABLE;
    ret->size = INITIAL_TABLE_SIZE;
    ret->entries = 0;
    ret->array = calloc(INITIAL_TABLE_SIZE, sizeof(closure *));
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
	printf("here.");
	prev->in->cons->car = value;
    } else {
	table->array[hash] = cons(list(2, symbol, value), chain);
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
    
    return car(assoc(symbol, chain));
}

symbol_table *table_union(symbol_table *a, symbol_table *b)
{
    symbol_table *ret = new_symbol_table();
    closure *item;
    int i;
    closure *chain;			            
    start_table_iteration(b, item, i){
	table_insert(car(item), second(item), ret);
    } end_table_iteration;

    start_table_iteration(a, item, i){
	table_insert(car(item), second(item), ret);
    } end_table_iteration;
    
    return ret;
}


#endif
