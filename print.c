#include "eight.h"

void print_closure(closure *a)
{
     if (a->type == SYMBOL) {
	  char* sym = symbol_id_to_string(a->symbol_id);
	  printf("%s", sym);
     } else if (a->type == FIXNUM) {
	  printf("%d", a->fixvalue);
     } else if (a->type == CHARACTER) {
          printf("%c", a->charvalue);
     } else if (a->type == NIL) {
	  printf("()");
     } else if (stringp(a)) {
          print_string(a);
     } else if (a->type == CONS_PAIR){
	  printf("(");
	  print_cons(a);
	  printf(")");
     } else if (a->type == NIL){
	  printf("()");
     }  else {
         printf("v%d", (int)a->value);
     }
     //printf("[");
     // print_assoc(a->closed);
     //printf("]");
}

void print_cons(closure *cons)
{
     print_closure(cons->cons->car);
     if (cdr(cons)->type == CONS_PAIR){
	  printf(" ");
	  print_cons(cons->cons->cdr);
     } else if (cdr(cons)->type == NIL){
     } else {
	  printf(" . ");
	  print_closure(cons->cons->cdr);
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

void print_assoc(closing *cl)
{
     if (cl != NULL){
         printf("%s->", symbol_id_to_string(cl->sym));
         print_closure(cl->val);
         printf(", ");
	 print_assoc(cl->next);
     }
}

void print_frame(frame *fm)
{
  printf("\nnext: ");
  print_cont(fm->next);
  printf("\nscope: ");
  print_assoc(fm->scope);
  printf("\nrib: ");
  print_assoc(fm->rib);
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
  if (a->type == NIL)
    return;
  printf("%c", car(a)->charvalue);
  print_string_internal(cdr(a));
}

void print_string(closure *a)
{
  printf("\"");
  print_string_internal(a);
  printf("\"");
}
