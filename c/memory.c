/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley (diiq, stm31415@gmail.com)

 This houses the memoy subsystem, the garbage collector, and
 associated testing routines.

***************************************************************************/

#ifndef EIGHT_MEMORY
#define EIGHT_MEMORY

#include "eight.h"

#define BLOCK_SIZE 1048576
#define MIN_SIZE   10

//----------------------------- STRUCTURES ----------------------------//


/* Eight's working memory is divided into blocks based on structure types.
   Each struct defined in eight.h has it's own linked list of memory blocks.

   In order to keep track of where free memory is available, a linked
   list of memory locations is kept with each memory block.  Each
   location represents a place where at least one struct's-worth of
   memory is available.

   This is all overkill for a reference-counting system; but if a precise
   collector is necessary (and it will be), we need a list of all the 
   objects in memory. This is the only part of the implementation I still feel 
   unsure of, so I hope this is not to appalling.
*/

typedef struct memory_block_struct memory_block;
typedef struct memory_location_struct memory_location;


struct memory_block_struct {
    memory_block *next;
    void *block;
    memory_location *free;
    void *end;
};


struct memory_location_struct {
    void *start;
    void *end;
    memory_location *next;
};


typedef struct {
    memory_block *operations;
    memory_block *frames;
    memory_block *machines;
    memory_block *cons_cells;
    memory_block *objects;
    memory_block *closures;
    memory_block *symbol_tables;
} memory_;

memory_ *memory;
//----------------------------- FUNCTIONS ----------------------------//


operation *new_operation();
frame *new_frame();
machine *new_machine();
cons_cell *new_cons_cell();
object *new_object();
closure *new_closure();
symbol_table *new_table();

void debug(char *a){
    if (DEBUG) printf("DEBUG: %s\n", a);
}


void free_obj_block(memory_location *freelist, void *start, int size) {
    // free_blocks assumes that all the blocks in a freelist
    // are of the same size.

    // start aligns with the end of one;
    // start+size aligns with the start of one;
    // start and start+size are all alone.
    if (freelist->start > start){
	memory_location *temp = malloc(sizeof(memory_location));
	temp->start = freelist->start;
	temp->end = freelist->end;
	temp->next = freelist->next;	\
	freelist->start = start;
	freelist->end = start+size;
	freelist->next = temp;
    } else {
	memory_location *pre = freelist;
	while (freelist->next &&
	       freelist->next->start < start) { 
	    pre = freelist;
	    freelist = freelist->next;
	}
	
	if (freelist->end == start) {
	    debug("Overlapping start.");
	    freelist->end = start + size;
	} else {
	    debug("No overlapping start.");
	    pre->next = malloc(sizeof(memory_location));
	    pre->next->next = freelist;
	    freelist = pre->next;
	    freelist->start = start;
	    freelist->end = start + size;
	}
    }	
    
    if (freelist->next &&
	freelist->next->start == freelist->end){
	debug("Overlapping end.");
	memory_location *temp = freelist->next;
	freelist->end = temp->end;
	freelist->next = temp->next;
	free(temp);
    }
}


void free_obj(memory_block *mem, void *obj, int size){

    while (mem && !(mem->block <= obj && mem->end > obj)) {
	mem = mem->next;
    }
    if (!mem->free){
	debug("first free block.");
	mem->free = malloc(sizeof(memory_location));
	mem->free->next = NULL;
	mem->free->start = obj;
	mem->free->end = obj + size;
    } else {
	free_obj_block(mem->free, obj, size);
    }	
    if (mem->free->start == mem->block &&
	mem->free->end == mem->end &&
	mem->next) {
	debug("Freeable block.");
	free(mem->free);
	mem->free = mem->next->free;
	free(mem->block);
	mem->block = mem->next->block;
	mem->end = mem->next->end;
	memory_block *temp = mem->next;
	mem->next = mem->next->next;
	free(temp);
    }   
}


#define new_allocator(type, num_per_block)                  	\
    void free_ ## type (type *a)				\
    {								\
	free_obj(memory->type ## s, (void *)a, sizeof(type));	\
    }								\
								\
    void *new_ ## type ## _block()				\
    {								\
	void * nblock = malloc(sizeof(type) * num_per_block);	\
	memory_block *  new = malloc(sizeof(memory_block));	\
	new->next = memory->type ## s;				\
	new->block = nblock;					\
	new->end = nblock + sizeof(type) * num_per_block;	\
	new->free = malloc(sizeof(memory_location));		\
	new->free->next = NULL;					\
	new->free->start = new->block+sizeof(type);		\
	new->free->end = new->end;				\
	memory->type ## s = new;				\
	return nblock;						\
    }								\
								\
    type *new_ ## type ## _space(){					\
	void *ret;						\
	memory_block *curr = memory->type ## s;			\
	while (!curr->free && curr->next) curr = curr->next;	\
	if (curr->free) {					\
	    memory_location * ffree = curr->free;		\
	    ret = ffree->start;					\
	    ffree->start += sizeof(type);			\
	    if (ffree->start == ffree->end){			\
		memory_location *temp = curr->free->next;	\
		free(curr->free);				\
		curr->free = temp;				\
	    }							\
	} else {						\
	    printf("whoops, new block needed!\n");		\
	    ret = new_ ## type ## _block();			\
	}							\
	return (type *) ret;					\
    }								\
									\
    void initialize_ ## type ## _memory() {				\
	memory->type ## s = malloc(sizeof(memory_block));		\
	memory->type ## s->block = malloc(sizeof(type)*num_per_block);	\
	memory->type ## s->next = NULL;					\
	memory->type ## s->end = memory->type ## s->block +		\
	    sizeof(type)*num_per_block;					\
	memory->type ## s->free = calloc(sizeof(memory_location), 1);	\
	memory->type ## s->free->start = memory->type ## s->block +	\
	    sizeof(type);						\
	memory->type ## s->free->end =  memory->type ## s->block +	\
	    sizeof(type)*num_per_block;					\
	memory->type ## s->free->next = NULL;				\
    }									\
    

new_allocator(operation, 100);
new_allocator(frame, 50);
new_allocator(machine, 10);
new_allocator(cons_cell, 1000);
new_allocator(object, 1000);
new_allocator(closure, 1000);
new_allocator(symbol_table, 100);


closure *the_nil;

void init_memory() {
    memory = malloc(sizeof(memory_));		
    initialize_operation_memory();
    initialize_frame_memory();
    initialize_machine_memory();
    initialize_cons_cell_memory();
    initialize_object_memory();
    initialize_closure_memory();
    initialize_symbol_table_memory();
    
    the_nil = new_closure_space();
    the_nil->type = NIL;
}

    // Not sure I like this, but it will stick around for now
closure *nil()
{
    return the_nil;
}

//--------------------------------- TESTING --------------------------------//

/* void test_initialize() */
/* { */
/*     printf("here we go...\n"); */
/*     initialize_closure_memory(); */
/* } */

/* void test_new_block() */
/* { */
/*     closure *a = new_closure_block(); */
/*     printf("block made, size %li * 10, at %p.\n",  */
/* 	   sizeof(closure), memory->closures->block); */
/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */
/*     printf("%p\n", memory->closures->next); */
    
/*     free_closure(a); */
/*     printf("The next block was freed: %p\n", memory->closures->next); */
/* } */


/* void test_new_obj() */
/* { */
/*     closure * a = new_closure_block(); */
/*     closure * b = new_closure_space(); */
/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */
   
/*     free_closure(a); */
/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */

/*     free_closure(b); */
/*     printf("The next block was freed: %p\n", memory->closures->next); */
/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */

/* } */


/* void test_new_obj_makes_block() */
/* { */
/*     printf("lotsandlots\n"); */
/*     closure * a = new_closure_space(); */
/*     closure * b = new_closure_space(); */
/*     closure * c = new_closure_space(); */
/*     closure * d = new_closure_space();     */
/*     closure * e = new_closure_space(); */
/*     closure * f = new_closure_space(); */
/*     closure * g = new_closure_space(); */
/*     closure * h = new_closure_space(); */
/*     closure * i = new_closure_space(); */
/*     closure * j = new_closure_space(); */

/*     free_closure(c); */

/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */
   
/*     printf("things!\n"); */
/*     free_closure(j); */
/*     printf("free start: %p\n",// free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start//, memory->closures->free->end, */
/* 	   //memory->closures->block, memory->closures->end */
/* 	   ); */
   
/*     printf("things!\n"); */
/*     free_closure(i); */
/*     printf("free start: %p\n free end:%p\n block start: %p\n block end: %p\n",  */
/* 	   memory->closures->free->start, memory->closures->free->end, */
/* 	   memory->closures->block, memory->closures->end); */
   
/*     printf("things!\n"); */
/*     free_closure(h); */
/*     free_closure(g); */

/* } */

/* int main(){ */
/*     test_initialize(); */
/*     test_new_block(); */
/*     test_new_obj(); */
/*     test_new_obj_makes_block(); */
/*     return 0; */
/* } */



#endif
