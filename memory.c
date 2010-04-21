/* OK, the problem right now is that the references in teh c stack */
/* don't match the ones in the eight stack; so what I need to do is only */
/* collect at the ends of eight steps; and in the meantime have an */
/* accordion of some sort. */
// The clearest route seems to be make a 'next' function that returns the
// next memory location; then have, I guess, a linked list of memory chunks;
// all of equal size? logarithmic size? I think equal size, though how
// big is still an open question. I only traverse linearly and in one
// direction, so a linked list ought to work fine.

#include "eight.h"

#define BLOCK_SIZE 1048576

memory *memory_a;
memory *memory_b;
int garbage_check = 4;

machine * init_memory()
{
     // Initialize memory with the root object, a machine,
     // and return the machine.
     memory_a = malloc(sizeof(memory));
     memory_a->first = calloc(1, sizeof(memory_block));
     memory_a->first->this = calloc(1, BLOCK_SIZE);

     memory_a->current = malloc(sizeof(memory_location));
     memory_a->current->block = memory_a->first;
     memory_a->current->offset = 0;
     memory_a->size = 0;

     memory_a->from = malloc(sizeof(memory_location));

     machine *memory_root = allocate(sizeof(machine));
     memory_root->type = MACHINE;

     return memory_root;
}

void *current_location(memory *m) 
{
     return (m->current->block->this+m->current->offset);
}

void *from_location(memory *m) 
{
     return (m->from->block->this+m->from->offset);
}

void next_reference(memory *m)
{      
     //    printf("read from %d to %d\n", 
//	    m->from->offset, 
//	    mysizeof(from_location(m))+m->from->offset);
     if (m->from->offset == -1) // this means we're at the end of the heap
	  return;

     m->from->offset += mysizeof(from_location(m));
     
     if(*(obj_type *)from_location(m) == 0 ||
	  m->from->offset >= BLOCK_SIZE){ // there's nothing left 
	                                     //in this block...
	  if (m->from->block->next != NULL){
	       // if there's another block,
	       m->from->block = m->from->block->next;
	       m->from->offset = 0;
	  } else {
	       m->from->offset = -1; //we're out of heap.
	  }
     }	  
}

void *allocatery(memory *mo, int size)
{

  //     printf("wrote to %d to %d\n", mo->current->offset, mo->current->offset+size);

     if ((mo->current->offset + size) >= BLOCK_SIZE){
	  memory_block* block = calloc(1, sizeof(memory_block));
	  block->this = calloc(1, BLOCK_SIZE);
	  mo->current->block->next = block;
	  mo->current->block = block;
	  mo->current->offset = 0;
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
     if (type == NIL ||
	 type == CONS_PAIR ||
	 type == NUMBER || 
	 type == INTERNAL || 	 
	 type == CHARACTER ||
	 type == SYMBOL ||
	 type == CONTINUATION ||
	 type == BUILTIN || 
	 type == C_OBJECT){
	  return sizeof(closure);
     } else if (type == MACHINE_FLAG ||
		type == CLOSURE_OP){
	  return sizeof(operation);
     } else if (type == MACHINE) {
	  return sizeof(machine);
     } else if (type == FRAME) {
	  return sizeof(frame);
     } else if (type == CONS_CELL) {
	  return sizeof(cons_cell);
     } else if (type == REFERENCE) {
	  return ((memory_reference*)typed)->size;
     }
     // printf("Whoah, nelly. Unknown type to garbage collect:%i\n", type);
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
//     print_type(*(obj_type*)ref);
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


machine * collect()
{
     if (memory_a->size < garbage_check){
	  return memory_a->first->this;
     }
     //printf("collectin', 'cause size is %d and check is %d.\n",
     //	    memory_a->size,
     //	    garbage_check);
     //print_heap(memory_a);
     printf("collecting\n");     // allocate b
     memory_b = malloc(sizeof(memory));
     memory_b->first = malloc(sizeof(memory_block));
     memory_b->first->this = calloc(1, BLOCK_SIZE);
     memory_b->current = malloc(sizeof(memory_location));
     memory_b->current->block = memory_b->first;     
     memory_b->current->offset = 0;

     memory_b->from = malloc(sizeof(memory_location));
     memory_b->from->block = memory_b->first;
     memory_b->from->offset = 0;
     memory_b->size = 0;
     // copy machine
     repair_reference(memory_a->first->this);
     // collectify
     collectify();
          
     // free old memory
     free_memory(memory_a);
     memory_a = memory_b;
     // printf("collected\n");     
     // return new machine
     //     print_heap(memory_a);
     garbage_check = memory_a->size * 2;
     return memory_a->first->this;
}

void collectify()
{
     if (memory_b->from->offset == -1)
	  return;
     
     void *location = from_location(memory_b);
     obj_type type = *(obj_type*)(location);
    if (type == NIL       ||	
	type == NUMBER    || 
	type == INTERNAL  || 	 
	type == CHARACTER ||
	type == SYMBOL    ||
	type == BUILTIN   || 
	type == C_OBJECT) {
      	 // printf("collecting a non-reference closure\n");
	 closure *it =  ((closure *)(location));
	 it->closing = repair_reference(it->closing);
	 it->info = repair_reference(it->info);
    } else if (type == CONS_PAIR) {
       // printf("collecting a cons pair\n");
	 closure *it =  ((closure *)(location));
	 it->cons = repair_reference(it->cons);
	 it->closing = repair_reference(it->closing);
	 it->info = repair_reference(it->info);
    } else if (type == CONS_CELL) {
      // printf("collecting a cell\n");
	 cons_cell *it = ((cons_cell *)(location));
	 it->car = repair_reference(it->car);
	 it->cdr = repair_reference(it->cdr);
    } else if (type == CONTINUATION) {
	 closure *it = ((closure *)(location));
	 it->mach = repair_reference(it->mach);
	 it->closing = repair_reference(it->closing);
	 it->info = repair_reference(it->info);	 
    } else if (type == MACHINE){
       // printf("collecting a machine\n");
	 machine *it = ((machine *)(location));
	 it->current_frame = repair_reference(it->current_frame);
	 it->base_frame = repair_reference(it->base_frame);
	 it->accum = repair_reference(it->accum);
    } else if (type == FRAME){
       // printf("collecting a frame\n");
	 frame *it = ((frame *)(location));
	 it->next = repair_reference(it->next);
	 it->rib = repair_reference(it->rib);
	 it->scope = repair_reference(it->scope);
	 it->signal_handler = repair_reference(it->signal_handler);
	 it->below = repair_reference(it->below);
    } else if (type == CLOSURE_OP ||
	       type == MACHINE_FLAG){
	 // printf("collecting an operation\n");
	 operation *it = ((operation *)(location));
	 it->closure = repair_reference(it->closure);
	 it->next = repair_reference(it->next);
    } else {
	 // printf("type: %d from %d to ?", type, memory_b->from->offset);

	 return;
    }
    next_reference(memory_b);
    collectify();
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
     } else {
       printf("WTF is a %d?", type);
     }
}

void print_heapi(memory *a)
{
     if (a->from->offset == -1) {	 
	 printf("end of heap\n");
	 return;
    }
     obj_type type = *(obj_type*)from_location(a);
     print_type(type);
     if (type == NUMBER){
	  print_closure(from_location(a));
     }
     printf(" ");
     next_reference(a);
     print_heapi(a);
}

void print_heap(memory *a){
     a->from->block = a->first;
     a->from->offset = 0;
     printf("start of heap\n");
     print_heapi(a);
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
