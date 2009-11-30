#ifndef COMMANDS_
#define COMMANDS_

#include "eight.h"
#include <dlfcn.h>

//what actually needs to be done? the symbol needs to be registered, 
// the lambda-list built, the arguments looked up, function made, and
// function bound.

#define make_arg(sym) symbol(string_to_symbol_id(#sym))
#define get_arg(sym, m) looker_up(symbol(string_to_symbol_id(#sym)),	\
					m->current_frame,		\
					m->base_frame)    		\

#define intern_fn(fn_name, fn_pointer, lambda_list, m)	do {		\
	  sym = symbol(string_to_symbol_id(#fn_name));			\
	  sym->type = INTERNAL;						\
	  sym->builtin_fn = fn_pointer;					\
	  internal_set(symbol(string_to_symbol_id(#fn_name)),		\
		       cons(lambda_list, cons(sym, nil())),		\
		       m->current_frame,				\
		       m->base_frame);					\
     } while (0)

 
void set_fn(machine *m)
{
     closure *a = get_arg(a, m);
     if (a->type != SYMBOL){
	  closure *sig = build_signal(cons(string("\n\nwere I holding a clay pot\nI could call the mouth\nan umbilical cord\n o mother-universe\n where is the child born in the kiln-fire?\n\n\nerror: I attempted to set! something that isn't a symbol: "), a), m);
	  toss_signal(sig, m);
     } else {
     closure *b = get_arg(b, m);
     internal_set(a, b, m->current_frame, m->base_frame);
     m->accum = b;
     }
}



int symbol_eq(closure *a, closure *b){
     if ((a->type == SYMBOL) && (b->type == SYMBOL) &&
	 (a->symbol_id == b->symbol_id))
	  return 1;
     return 0;
};

int fixnum_eq(closure *a, closure *b){
     if ((a->type == FIXNUM) && (b->type == FIXNUM) &&
	 (a->fixvalue == b->fixvalue))
	  return 1;
     return 0;
};


void is_fn(machine *m)
{
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);

     if ((a == b) || 
	 fixnum_eq(a, b) || 
	 symbol_eq(a, b) ||  // still uncertain if this is the right thing.
	 ((a->type == NIL) && (b->type == NIL))
	 ){
	  m->accum = symbol(T);
     } else {
	  m->accum = nil();
     }
}

void oif_fn(machine *m)
{
     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     newop->type = CLOSURE_OP;
     newop->next = m->current_frame->next;
     m->current_frame->next = newop;

     closure *test = get_arg(test, m);
     closure *then = get_arg(then, m);
     closure *elser = get_arg(elser, m);

     if (test->type != NIL) {
	  newop->closure = then;
     } else {
	  newop->closure = elser;
     }
}

void cons_fn(machine *m)
{
     closure *a = get_arg(car, m);
     closure *b = get_arg(cdr, m);
     m->accum = cons(a, b);
}

void car_fn(machine *m)
{
     closure *acons = get_arg(cons, m);
     if (acons->type != CONS_PAIR && acons->type != NIL){
	  closure *sig = build_signal(cons(string("\n\nfinding the thousand things\nreflected within the tao\nis beyond me.\n\n\nerror: I attempted to retrieve the car from something that was not a cons pair: "), acons), m);
	  toss_signal(sig, m);
     } else {
	  if(acons->type == NIL){
	       m->accum = nil();
	  } else {
	       m->accum = car(acons);
	  }
     }
}

void cdr_fn(machine *m)
{
     closure *acons = get_arg(cons, m);
     if (acons->type != CONS_PAIR && acons->type != NIL){
	  closure *sig = build_signal(cons(string("\n\nleaves rot on the warm dirt\nwhile the tree drinks rain\n he is not naked\n\n\nerror: I attempted to retrieve the cdr from something that is not a cons pair: "), acons), m);
	  toss_signal(sig, m);
     } else {
	  if(acons->type == NIL){
	       m->accum = nil();
	  } else {
	       m->accum = cdr(acons);
	  }
     }
}

void atomp_fn(machine *m)
{
     closure *a = get_arg(a, m);
     if (a->type != CONS_PAIR){
	  m->accum = a;
     } else {
	  m->accum = nil();
     }
}

void print_fn(machine *m)
{
     closure *a = get_arg(a, m);
     print_closure(a);
     printf("\n");
}

void prmachine_fn(machine *m)
{
    print_stack(m->current_frame);
    print_stack(m->base_frame);
}

void leak_fn(machine *m)
{
     closure *sym = get_arg(sym, m);
     if (sym->type != SYMBOL && sym->type != NIL){
	  closure *sig = build_signal(cons(string("\n\nI cannot speak about rhymes\nwhen the juice of the orange\nis still dripping from my chin\n\n\nerror: I attempted to leak something that isn't a symbol: "), sym), m);

	  print_closure(sym);
	  printf("\n");
	  //print_closure((closure *) 5);
	  toss_signal(sig, m);
     } else {
	  closure *clos = get_arg(closure, m);
	  closure *ret = (closure *)GC_MALLOC(sizeof(closure));
	  ret->type = clos->type;
	  ret->value = clos->value;
	  ret->annotation = clos->annotation;
	  ret->closed = remove_sym(sym->symbol_id, clos->closed);  
	  m->accum = ret;
     }
}

void comma_fn(machine *m)
{     
     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     closure *clos = get_arg(closure, m);
     newop->next = m->current_frame->next;
     newop->type = CLOSURE_OP;
     newop->closure = clos;
     m->current_frame->next = newop;
}

void callcc_fn(machine *m)
{    
     closure *fn = get_arg(fn, m);
     closure *ret = (closure *)GC_MALLOC(sizeof(closure));
     ret->type = CONTINUATION;
     ret->mach = stack_copy(m);
     // (quote ((a) ret))
     closure *cont = cons(quote(),
			    cons(cons(cons(make_arg(a),  nil()),
				      cons(ret, nil())),
				 nil()));
     closure *call = cons(fn, cons(cont, nil()));

     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     newop->next = m->current_frame->next;
     newop->type = CLOSURE_OP;
     newop->closure = call;
     m->current_frame->next = newop;
}

frame *find_signal_handler(frame *f){
     if (f->signal_handler != NULL)
	  return f;
     if (f->below == NULL)
	  return NULL;
     return find_signal_handler(f->below);
}

void toss_signal(closure* sig, machine* m)
{
     frame *fm =  find_signal_handler(m->current_frame);

     if (fm == NULL) {
	  printf("No handler.\n");
	  m->current_frame = m->base_frame;
	  m->accum = sig; //cdr(car(cdr(sig)));
	  return;
     };
     m->current_frame = fm;
     closure *fn = m->current_frame->signal_handler;
     
     sig = cons(fn, cons(sig, nil()));

     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     newop->next = m->current_frame->next;
     newop->type = CLOSURE_OP;
     newop->closure = sig;
     m->current_frame->next = newop;
}

closure * build_signal(closure *a, machine *m)
{
     closure *ret = (closure *)GC_MALLOC(sizeof(closure));
     ret->type = CONTINUATION;
     ret->mach = stack_copy(m);
     // (quote ((a) ret))
     closure *cont = cons(cons(make_arg(a),  nil()),
			  cons(ret, nil()));
     cont = cons(cont, cons(a, nil()));
     cont = cons(quote(), cons(cont, nil()));
     return cont;
}

void signal_fn(machine *m)
{
     closure *a = get_arg(sig, m);
     toss_signal(build_signal(a, m), m);
}

void unhandle_signal(machine *m)
{
     closure *a = get_arg(sig, m);
     m->current_frame = m->current_frame->below->below; // to remove the current handler
     toss_signal(cons(quote(), cons(a, nil())), m);
}

void add_handler(machine *m){     
     closure *handler = get_arg(handler, m);
     m->current_frame->signal_handler = handler;

     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     closure *body = get_arg(body, m);
     newop->next = m->current_frame->next;
     newop->type = CLOSURE_OP;
     newop->closure = body;
     m->current_frame->next = newop;
}
void base_handler(machine *m){     
     closure *handler = get_arg(handler, m);
     m->base_frame->signal_handler = handler;
}



//------------------ MATHS!----------------------//


void plus_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = fixnum(a->fixvalue + b->fixvalue);
}
void minus_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = fixnum(a->fixvalue - b->fixvalue);
}
void multiply_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = fixnum(a->fixvalue * b->fixvalue);
}
void divide_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = fixnum(a->fixvalue / b->fixvalue);
}
void greater_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     if (a->fixvalue > b->fixvalue)
       m->accum = symbol(T);
     else 
       m->accum = nil();
}
void less_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     if (a->fixvalue < b->fixvalue)
       m->accum = symbol(T);
     else 
       m->accum = nil();
}

//------------------------------- FFI --------------------------------//


/* char *to_c_string(closure *str); */

/* closure *to_eight_string(char *str); */

/* int fix_to_int(closure *num); */

/* closure *int_to_fix(int num); */

/* void load_lib_fn(machine *m){ */
/*   closure *name = get_arg(name, m); */
/*   char *rname = to_c_string(name); */
/*   void *lib = dlopen(rname,RTLD_NOW|RTLD_GLOBAL); */
/*   closure *ret = (closure *)GC_MALLOC(sizeof(closure)); */
/*   ret->type = C_OBJECT; */
/*   ret->value = lib; */
/*   m->accum = ret; */
/* } */

/* void get_lib_object_fn(machine *m){ */
/*   closure *lib = get_arg(lib, m); */
/*   closure *name = get_arg(name, m); */
/*   char *rname = to_c_string(name); */
/*   void *obj = dlsym(rname, (lib->value)); */
/*   closure *ret = (closure *)GC_MALLOC(sizeof(closure)); */
/*   ret->type = C_OBJECT; */
/*   ret->value = &obj; */
/*   m->accum = ret; */
/* } */

/* void call_c_fn(machine *m){ */
/*   closure *fn = get_arg(fn, m); */
/*   closure *args = get_arg(args, m); */
/*   int len = length(args); */
/*   void * (*pooer)() = (fn->value); */
/*   void *rret; */
/*   switch (len) { */
/*   case 0: rret = &(pooer()); */
/*   } */
/*   closure *ret = (closure *)GC_MALLOC(sizeof(closure)); */
/*   ret->type = C_OBJECT; */
/*   ret->value = rret; */
/*   m->accum = ret; */
/* } */

//--------------------------------------------------------------------//


void new_basic_commands(machine *m)
{	  
    // TODO This is still more complicated than it should be.
     closure *sym;
     
     intern_fn(is, &is_fn, cons(make_arg(a), 
				cons(make_arg(b), nil())), m);
     intern_fn(set!, &set_fn, cons(cons(symbol(QUOTE), 
					cons(make_arg(a), nil())), 
				   cons(make_arg(b), nil())), m);
     intern_fn(oif, &oif_fn, cons(
		    make_arg(test), //arg
		    cons( //cons
			 cons(symbol(QUOTE), cons(make_arg(then), nil())),//arg
			 cons( //<--cons                  vv arg vv
			      cons(symbol(QUOTE), cons(make_arg(elser), nil())), 
			      nil()))), m); // nil

     intern_fn(cons, &cons_fn, cons(make_arg(car), 
				cons(make_arg(cdr), nil())), m);

     intern_fn(car, &car_fn, cons(make_arg(cons), nil()), m);

     intern_fn(cdr, &cdr_fn, cons(make_arg(cons), nil()), m);

     intern_fn(atom-p, &atomp_fn, cons(make_arg(a), nil()), m);

     intern_fn(print, &print_fn, cons(make_arg(a), nil()), m);

     intern_fn(leak, &leak_fn, cons(make_arg(sym), 
				cons(make_arg(closure), nil())), m);
        
     intern_fn(comma, &comma_fn, cons(make_arg(closure),  nil()), m);
   
     intern_fn(prmachine, &prmachine_fn, nil(), m);

     intern_fn(call/cc, &callcc_fn, cons(cons(symbol(QUOTE), cons(make_arg(fn),  nil())), nil()), m);

     intern_fn(signal, &signal_fn, cons(make_arg(sig),  nil()), m);

     intern_fn(handle-signals, &add_handler, cons(cons(symbol(QUOTE), cons(make_arg(handler), nil())),
						cons(cons(symbol(QUOTE), 
							  cons(make_arg(body), nil())), nil())), m);  
     intern_fn(base-signal-handler, &base_handler, cons(cons(symbol(QUOTE), 
							    cons(make_arg(handler), nil())), 
						       nil()), m);  

     intern_fn(unhandle-signal, &unhandle_signal, cons(make_arg(sig),  nil()), m);

     //maths

     intern_fn(plus, &plus_fn, cons(make_arg(a), 
				    cons(make_arg(b), nil())), m);
     intern_fn(minus, &minus_fn, cons(make_arg(a), 
				      cons(make_arg(b), nil())), m);
     intern_fn(multiply, &multiply_fn, cons(make_arg(a), 
					    cons(make_arg(b), nil())), m);
     intern_fn(divide, &divide_fn, cons(make_arg(a), 
					cons(make_arg(b), nil())), m);
     intern_fn(>, &greater_fn, cons(make_arg(a), 
					cons(make_arg(b), nil())), m);
     intern_fn(<, &less_fn, cons(make_arg(a), 
					cons(make_arg(b), nil())), m);
}    

#endif
