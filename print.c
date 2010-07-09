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

#include "eight.h"

void print_closure(closure *a)
{
     if (a->in->type == SYMBOL) {
	  wchar_t* sym = symbol_id_to_string(a->in->symbol_id);
	  printf("%ls", sym);
     } else if (a->in->type == NUMBER) {
	  printf("%d", a->in->num);
     } else if (a->in->type == CHARACTER) {
          printf("$%c", a->in->character);
     } else if (a->in->type == NIL) {
	  printf("()");
     } else if (stringp(a)) {
          print_string(a);
     } else if (a->in->type == CONS_PAIR){
	  printf("(");
	  print_cons(a);
	  printf(")");
     } else if (a->in->type == NIL){
	  printf("()");
     }  else {
	 printf("v%p", a->in->obj);
     }
    
     //  if(!nilp(a->closing)){
     // printf("[");
     //  print_closure(a->closing);
     // printf("]");
     //}
}

void print_cons(closure *cons)
{
     print_closure(cons->in->cons->car);
     if (cdr(cons)->in->type == CONS_PAIR){
	  printf(" ");
	  print_cons(cons->in->cons->cdr);
     } else if (cdr(cons)->in->type == NIL){
       //printf("()");
     } else {
	  printf(" . ");
	  print_closure(cons->in->cons->cdr);
     }
}

void print_op(operation* ins)
{
     printf("\n    type: %d ", ins->type);
     if (ins->type == MACHINE_FLAG)
	  printf("%d ", ins->flag);
     print_closure(ins->closure);
};

void print_cont(operation *ins)
{
     if(ins != NULL){
	  print_op(ins);
	  print_cont(ins->next);
     }
}

void print_frame(frame *fm)
{
  printf("\nnext: ");
  print_cont(fm->next);
  printf("\nscope: ");
  print_closure(fm->scope);
  printf("\nrib: ");
  print_closure(fm->rib);
}

void print_machine(machine *m){
  print_stack(m->current_frame);
}

void print_stack(frame *fm)
{
  if (fm != NULL){
    print_frame(fm);
    printf("\n^_________________^");
    print_stack(fm->below);
  }
}
      
void print_string_internal(closure *a)
{
  if (a->in->type == NIL)
    return;
  printf("%c", car(a)->in->character);
  print_string_internal(cdr(a));
}

void print_string(closure *a)
{
  printf("\"");
  print_string_internal(a);
  printf("\"");
}




/// TODO: asking for a stack trace should return an list, so that it
/// can be manipulated and printed as-you-please.

void print_stack_trace_i(frame *f, frame *base_frame)
{
    if (f->below != NULL){
	print_stack_trace_i(f->below, base_frame);
	if (f->below->below != NULL){
	    	printf("called ");
	}
        print_frame_trace(f, base_frame);
    }
}

void print_stack_trace(machine *m)
{
    print_stack_trace_i(m->current_frame, m->base_frame);
}


void print_frame_trace(frame *f, frame *base_frame)
{
    if (!nilp(f->function)){
	closure *name = cheap_car(assoc(symbol(FUNCTION_NAME), 
					f->function->in->info));
	print_closure(name);
    }
	printf(", with:\n");
	print_scope(f->rib, base_frame);
    
}

void print_scope(closure *scope, frame *base_frame)
{
    if (!nilp(scope)){
	printf("        ");
	print_closure(car(scope));
	printf("\n");
	print_scope(cdr(scope), base_frame);
    }
}
		 




	    // print_chik, with chik = (5 5),
	    //                  booper = (() square), called:
	    // print, with ((a 25)), called:
	    // hooper, which signaled!

 	
