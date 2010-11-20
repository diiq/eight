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

   Within each block, the order of locations does not matter.

   This makes allocating memory O(N) on the number of blocks; and by
   keeping track of not just the first block on the chain, but the
   first block with available memory, makes both allocating and
   freeing O(1).

   Do I want separate structs for each type of memory?
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
    memory_block *tables;
} memory_;

memory_ *memory;
//----------------------------- FUNCTIONS ----------------------------//


operation *new_operation();
frame *new_frame();
machine *new_machine();
cons_cell *new_cons_cell();
object *new_object();
closure *new_closure();
//table *new_table();



void free_obj_block(memory_location *freelist, void *start, int size) {
    // free_blocks assumes that all the blocks in a freelist
    // are of the same size.

    // start aligns with the end of one;
    // start+size aligns with the start of one;
    // start and start+size are all alone.
    if (freelist->start > start){
	// start is before everything.
	if (freelist->end == start+size+1){
	    freelist->start = start;
	    return;
	} else {
	    memory_location * temp = malloc(sizeof(memory_location));
	    temp->start = freelist->start;
	    temp->end = freelist->end;
	    temp->next = freelist->next;
	    freelist->start = start;
	    freelist->end = start + size;
	    freelist->next = temp;
	}
    } else {
	while (freelist->next &&
	       freelist->start < start) 
	    freelist = freelist->next;
	
	if (freelist->end == start+1) {
	    freelist->end == start + size;
	} else {
	    memory_location * temp = malloc(sizeof(memory_location));
	    temp->start = freelist->start;
	    temp->end = freelist->end;
	    temp->next = freelist->next;
	    freelist->start = start;
	    freelist->end = start + size;
	    freelist->next = temp;
	}	
	
	if (freelist->next &&
	    freelist->next->start == freelist->end+1){
	    memory_location *temp = freelist->next;
	    freelist->end = temp->end;
	    freelist->next = temp->next;
	    free(temp);
	}
    }
}

void free_obj(memory_block *mem, void *obj, int size){
    memory_block *pre;
    while (mem && !(mem->block <= obj && mem->end > obj)) {
	pre = mem;
	mem = mem->next;
    }
    free_obj_block(mem->free, obj, size);
    if (mem->free->start == mem->block &&
	mem->free->end == mem->end) {
	free(mem->free);
	free(mem->block);
	pre->next = mem->next;
	free(mem);
    }
}

void *new_closure_block(){					
    void * nblock = malloc(sizeof(closure) * 10);			
    memory_block *  new = malloc(sizeof(memory_block));		
    new->next = memory->closures;				
    new->block = nblock;
    new->end = nblock + sizeof(closure) * 10;
    new->free = malloc(sizeof(memory_location));
    new->free->next = NULL;
    new->free->start = new->block;
    new->free->end = new->end;
    memory->closures = new;					
    free_obj(memory->closures, 
	     nblock+sizeof(closure), 
	     sizeof(closure) * (10-1));
    return nblock;
}								
								
closure *new_closure(){						
    void *ret;							
    memory_block *curr = memory->closures;			
    while (!curr->free && curr->next) curr = curr->next;	
    if (curr->free) {						
	memory_location * ffree = curr->free;			
	ret = ffree->start;					
	ffree->start += sizeof(closure);				
	if (ffree->start == ffree->end){			
	    memory_location *temp = curr->free->next;	
	    free(curr->free);				
	    curr->free = temp;
	    }							
	} else {					       
	    ret = new_closure_block();	
	    memory->closures->free->start += sizeof(closure);
	}						
	return (closure*) ret;				
    }							
									
									    
    

//new_allocator(closure, 10);

initialize_memory(){
    memory = malloc(sizeof(memory_));
    printf("a memory structure...\n");
    memory->closures = malloc(sizeof(memory_block));
    memory->closures->block = malloc(sizeof(closure)*10);
    memory->closures->next = NULL;

    memory->closures->free = calloc(sizeof(memory_location), 1);
    memory->closures->free->start = memory->closures->block + sizeof(closure);
    memory->closures->free->end =  memory->closures->block + sizeof(closure)*10;
    memory->closures->free->next = NULL;

    printf("location...\n");

    new_closure_block();
    printf("block made!");

    //new_closure_block();
    //printf("block made!");
    //new_closure_block();
    //printf("block made!");

}

//--------------------------------- TESTING --------------------------------//

void test_allocator(){
    printf("here we go...\n");
    initialize_memory();
}

int main(){
    test_allocator();
    printf("%d", (int)memory->closures->next);
    return 0;
}



#endif
