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
#define MIN_SIZE   2
//----------------------------- STRUCTURES ----------------------------//

/* A memory block is a big block of memory. Astonishing, right? Memory
blocks form a linked list; block->this is the actual address of the
first object stored in the block. block->next is the next block in the
list. Memory blocks are BLOCK_SIZE bytes. */ 

typedef struct memory_block_struct memory_block;

struct memory_block_struct {
     void *this;
     memory_block *next;
};



/* A memory location is a pairing of a memory block and an integer
offset into that block. Adding block->this and offset will point to
the object at this memory location. */

typedef struct {
     memory_block *block;
     int offset;
} memory_location;



/* A memory reference is a ghost object; it only exists during garbage
collection, when it is used to keep track of the new location of an
object. BEWARE that you do not make the size of this struct larger
than any struct that might be stored in garbage-collected memory!
References MUST be the smallest structure in memory (or tied for that
honor). */

typedef struct {
     obj_type type;
     void *point;
     int size;
} memory_reference;



/* A memory must contain the first block in a linked list of memory
blocks, the 'current' memory location (the location that is being
written to), the 'from' memory location (the location that is being
read from), and the size (for keeping track of when next to collect
the garbage.  */

typedef struct {
     memory_block *first;
     memory_location *current;
     memory_location *from;
     int size; // this is in units of BLOCK_SIZE
} memory;


//----------------------------- GLOBALS ----------------------------//

// memory_a is the working memory
memory *memory_a;

// memory_b is where we copy to, when we garbage collect
memory *memory_b;

// garbage_check is a threshold; when memory->a > garbage_check
// then collection should happen.
int garbage_check = 3;

// the_nil is a constant reverence to () (there is only one nil)
closure *the_nil;

//--------------------------- SIGNATURES --------------------------//

machine *collect();

void *allocatery(memory *mo, int size);

void *repair_reference(void *ref);

void collectify();

memory *make_memory();

void copy_memory(int start, int end);

void free_memory(memory *a);

int mysizeof(void *typed);

void print_type(obj_type type);

void print_heap(memory *m);

void test_memory();

//--------------------------- FUNCTIONS --------------------------//


machine * collect()
{
    // Collect returns a pointer to a machine structure.

    // If it's not time to collect yet (the size of memory is
    // less than the garbage_check), then just return the
    // same machine, uncollected.
    if (memory_a->size < garbage_check){
	return memory_a->first->this;
 
   } else {
    
	// memory_b is the temporary location to which we'll copy
	// the stuff we want to keep.
	memory_b = make_memory();
    
	// Here we'll copy the machine and nil over to the new
	// memory --- they are the root of the memory tree.
	repair_reference(memory_a->first->this);
	the_nil = repair_reference(the_nil);
    
	collectify();
    
	free_memory(memory_a);
	memory_a = memory_b;
	
	// Don't collect again until the memory doubles in size.
	// TODO: upper bound is exp. memory growth. This will eventually
	// fail.
	garbage_check = (memory_a->size*2)<MIN_SIZE ? MIN_SIZE : memory_a->size * 2;
	
	return memory_a->first->this;
    }
}
    

memory *make_memory()
{ 
    // Returns an initialized memory structure. 

    memory *amemory;
    amemory = malloc(sizeof(memory));

    // Initialize the 'first' block
    amemory->first = calloc(1, sizeof(memory_block));
    amemory->first->this = calloc(1, BLOCK_SIZE);
    amemory->first->next = NULL;
    
    // Initialize the 'current' location
    amemory->current = malloc(sizeof(memory_location));
    amemory->current->block = amemory->first;
    amemory->current->offset = 0;
    
    // Initialize the 'from' location
    amemory->from = malloc(sizeof(memory_location));
    amemory->from->block = amemory->first;
    amemory->from->offset = 0;

    // Initialize the size
    amemory->size = 1;

    return amemory;
}


machine * init_memory()
{
    // Initialize memory with the root object, a machine.
    // Make the nil.  
    // Return the machine.

    memory_a = make_memory();
    machine *memory_root = allocate(sizeof(machine));
    memory_root->type = MACHINE;

    // Make the () closure
    the_nil = allocate(sizeof(closure));
    the_nil->in = allocate(sizeof(doubleref));
    the_nil->type = DREF;
    the_nil->in->type = NIL;
    the_nil->in->info = the_nil;
    the_nil->closing = the_nil;

    return memory_root;
}

closure *nil()
{ 
      return the_nil;
};

void *current_location(memory *mem) 
{
     return (mem->current->block->this+mem->current->offset);
}

void *from_location(memory *mem) 
{
     return (mem->from->block->this+mem->from->offset);
}

void next_reference(memory *mem)
{      
    // This function points the from_location to the next object
    // in memory, allowing iteration over all objects.

     if (mem->from->offset == -1) // this means we're at the end of the heap
	  return;

     mem->from->offset += mysizeof(from_location(mem));
     
     if(*(obj_type *)from_location(mem) == 0 ||
	mem->from->offset >= BLOCK_SIZE-1){ 
	  // there's nothing left 
	  // in this block...
	  if (mem->from->block->next != NULL){
	       // if there's another block,
	       if(!mem->from->block->next->this){
		   error();
	       }
	       mem->from->block = mem->from->block->next;
	       mem->from->offset = 0;
	  } else {
	       mem->from->offset = -1; //we're out of heap.
	  }
     }	  
}

void *allocatery(memory *mo, int size)
{
     if ((mo->current->offset + size) >= BLOCK_SIZE-1){
	  memory_block* block = calloc(1, sizeof(memory_block));
	  block->this = calloc(1, BLOCK_SIZE);
	  mo->current->block->next = block;
	  mo->current->block = block;
	  mo->current->offset = 0;
	  mo->current->block->next = NULL;
	  mo->size = mo->size + 1;
	  return allocatery(mo, size);
     } else { 
	  void *ret = current_location(mo);
	  mo->current->offset += size;
	  return ret;
     }
}

void *allocate(int size)
{
     void * p = allocatery(memory_a, size);
     return p;
}


int mysizeof(void *typed)
{
     // sizeof is compile-time, and we need
     // runtime.
     obj_type type = *(obj_type*)typed;
     if (type == DREF){
	 return sizeof(closure);
     } else if (type == NIL||
	 type == CONS_PAIR ||
	 type == NUMBER    || 
	 type == INTERNAL  || 	 
	 type == CHARACTER ||
	 type == SYMBOL    ||
	 type == TABLE    ||
	 type == CONTINUATION ||
	 type == BUILTIN   || 
	 type == C_OBJECT){
	  return sizeof(doubleref);
     } else if (type == CONS_CELL) {
	 return sizeof(cons_cell); 
     } else if (type == MACHINE_FLAG ||
		type == CLOSURE_OP){
	 return sizeof(operation);
     } else if (type == MACHINE) {
	  return sizeof(machine);
     } else if (type == FRAME) {
	  return sizeof(frame);
     } else if (type == SYMBOL_TABLE) {
	  return sizeof(symbol_table);
     } else if (type == REFERENCE) {
	  return ((memory_reference*)typed)->size;
     }
     printf("Whoah, nelly. Unknown type to garbage collect:%i\n", type);
     error(); 
     return -1;
};

void * repair_reference(void *ref){
     // Moves object @ ref to new memory, unless
     // it has already been moved.
     if (ref == NULL) {
	  // printf("(null) ");
	  return NULL;
     }
     // print_type(*(obj_type*)ref);
     if (*(obj_type*)ref == REFERENCE){
	  return ((memory_reference*)ref)->point;
     } else {
	  void *a = allocatery(memory_b, mysizeof(ref));
	  memcpy(a, ref, mysizeof(ref));
	  ((memory_reference*)ref)->size = mysizeof(ref);
	  ((memory_reference*)ref)->type = REFERENCE;
	  ((memory_reference*)ref)->point = a;
	  return a;
     }
}
 
void free_memory(memory *a)
{
     memory_block *temp;
     while(a->first != a->current->block){
	 temp = a->first->next;
	 free(a->first->this);
	 free(a->first);
	 a->first = temp;
     }
     free(a->first->this);
     free(a->first);
     free(a->current);
     free(a->from);
     free(a);
}



void collectify()
{
    while (memory_b->from->offset != -1){
     void *location = from_location(memory_b);
     obj_type type = *(obj_type*)(location);
     //     print_heap(memory_b);
     if (type == DREF){
	 closure *it =  ((closure *)(location));
	 it->closing = repair_reference(it->closing);
	 it->in = repair_reference(it->in);

     } else if (type == NIL        ||	
		type == NUMBER     || 
		type == INTERNAL   || 	 
		type == CHARACTER  ||
		type == SYMBOL     ||
		type == BUILTIN    ||
		type == C_OBJECT) {
	 // printf("collecting a non-reference closure\n");
	 doubleref *it =  ((doubleref *)(location));
	 it->info = repair_reference(it->info);

     } else if 	(type == TABLE) {
	 // printf("collecting a non-reference closure\n");
	 doubleref *it =  ((doubleref *)(location));
	 it->info = repair_reference(it->info);
	 it->table = repair_reference(it->table);

     } else if 	(type == SYMBOL_TABLE) {
	 symbol_table *it =  ((symbol_table *)(location));
	 int i;
	 closure **tarray = calloc(it->size, sizeof(closure*));
	 for(i=0; i<it->size; i++){
	     tarray[i] = repair_reference(it->array[i]);
	 }
	 free(it->array);
	 it->array = tarray;
	 
     } else if (type == CONS_PAIR) {
	 //printf("collecting a cons pair\n");
	 doubleref *it =  ((doubleref *)(location));
	 it->info = repair_reference(it->info);
	 it->cons = repair_reference(it->cons);

     } else if (type == CONS_CELL) {
	 //printf("collecting a cell\n");
	  cons_cell *it = ((cons_cell *)(location));
	  it->car = repair_reference(it->car);
	  it->cdr = repair_reference(it->cdr);

     } else if (type == CONTINUATION) {
	 //printf("collecting a continuation\n");
	 doubleref *it =  ((doubleref *)(location));
	 it->info = repair_reference(it->info);	 
	 it->mach = repair_reference(it->mach);

     } else if (type == MACHINE){
	 //printf("collecting a machine\n");
	  machine *it = ((machine *)(location));
	  it->current_frame = repair_reference(it->current_frame);
	  it->base_frame = repair_reference(it->base_frame);
	  it->accum = repair_reference(it->accum);

     } else if (type == FRAME){
	 //printf("collecting a frame\n");
	  frame *it = ((frame *)(location));
	  it->next = repair_reference(it->next);
	  it->rib = repair_reference(it->rib);
	  it->scope = repair_reference(it->scope);
	  it->signal_handler = repair_reference(it->signal_handler);
	  it->function = repair_reference(it->function);
	  it->below = repair_reference(it->below);

     } else if (type == CLOSURE_OP ||
		type == MACHINE_FLAG){
	 //printf("collecting an operation\n");
	  operation *it = ((operation *)(location));
	  it->closure = repair_reference(it->closure);
	  it->next = repair_reference(it->next);

     } else {
	 //printf("type: %d from %d to ?", type, memory_b->from->offset);
	  return;
     }
     next_reference(memory_b);
    }
}

void print_type(obj_type type)
{
     if ( type == NIL) printf("nil!");
     else if ( type == BUILTIN) printf("builtin!");
     else if ( type == NUMBER) printf("number!");
     else if ( type == CONS_PAIR) printf("cons!");
     else if ( type == CONS_CELL) printf("cell!");
     else if ( type == INTERNAL) printf("internal!");
     else if ( type == SYMBOL) printf("symbol!");
     else if ( type == REFERENCE) printf("ref!");
     else if (type  == CHARACTER) printf("character!");
     else if (type == SYMBOL) printf("symbol!");
     else if (type == C_OBJECT  ||
	      type == CONTINUATION) {
	  printf("continuation!");
     } else if (type == MACHINE){
	  printf("machine!");
     } else if (type == FRAME){
	  printf("frame!");
     } else if (type == CLOSURE_OP ||
		type == MACHINE_FLAG){
	  printf("operation!");
     } else if (type == DREF){
	 printf("dref.");
     } else {
	  printf("WTF is a %d?", type);
     }
}

void print_heap(memory *a){
    memory_block *from = a->from->block;
    int fromoff = a->from->offset;
    a->from->block = a->first;
    a->from->offset = 0;
    printf("start of heap\n");
     
    while (a->from->offset != -1) {	 
	obj_type type = *(obj_type*)from_location(a);
	print_type(type);
	if (type == NUMBER){
	    print_closure(from_location(a));
	}
	printf(" ");
	next_reference(a);
    }

    printf("end of heap\n");
    a->from->block = from;
    a->from->offset = fromoff;

}


void test_memory(){
// should start as 
//machine! frame! number!5 nil! operation! symbol! cons! cell! nil! number!4 cons! cell! number!4 cons! number!2 cons! cell! number!2 cons! number!23 end of heap

//ends as:
// machine! frame! operation! cons! cell! nil! number!2 cons! cell! number!4 cons! cell! nil! end of heap

     machine *goo = init_memory();
     goo->base_frame = new(frame);
     closure * a = number(5);
     goo->base_frame->type = FRAME;
     goo->base_frame->next = new(operation);
     a = symbol(T);
     goo->base_frame->next->type = CLOSURE_OP;
     goo->current_frame = goo->base_frame;
     goo->base_frame->scope = nil();
     goo->base_frame->rib = nil();
     goo->base_frame->next->closure = cons(number(2), cons(number(4), cons(nil(), nil())));
     a = number(23);

     printf("one!\n");
     print_machine(goo);
     print_heap(memory_a);
     printf("two!\n");
     goo = collect();
     print_machine(goo);
     print_heap(memory_a);
     printf("three!\n");
     goo = collect();
     print_machine(goo);
     print_heap(memory_a);
}


#endif
