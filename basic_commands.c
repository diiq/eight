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

#ifndef COMMANDS_
#define COMMANDS_

#include "eight.h"

#define make_arg(sym) symbol(string_to_symbol_id(L""#sym))

#define get_arg(sym, m) car(looker_up(symbol(string_to_symbol_id(L""#sym)), \
				      m->current_frame))     		

#define intern_fn(fn_name, fn_pointer, lambda_list, m)	do {		\
    sym = new(closure);							\
    sym->in = new(doubleref);						\
    sym->in->type = BUILTIN;     					\
    sym->type = DREF;							\
    sym->in->builtin_fn = fn_pointer;					\
    sym->closing = nil();						\
    sym->in->info = nil();						\
    val = list(2, lambda_list, sym);					\
    val->in->info =							\
      list(1, list(2, symbol(FUNCTION_NAME),				\
		   symbol(string_to_symbol_id(L""#fn_name))));		\
    internal_set(symbol(string_to_symbol_id(L""#fn_name)),		\
		 val,							\
		 m->current_frame,					\
		 m->base_frame);					\
  } while (0)


void set_fn(machine *m)
{
     closure *a = get_arg(a, m);
     if (a->in->type != SYMBOL){
	  closure *sig = build_signal(cons(string(L"\n\nHolding a clay pot\nI could call the mouth of it an umbilical cord\n o mother-universe, where is the child that was born in the kiln-fire?\n\n\nerror: I attempted to set! something that isn't a symbol: "), cons(a, nil())), m);
       	  toss_signal(sig, m);
     } else {
	 a = symbol(a->in->symbol_id);
	 closure *b = get_arg(b, m);
	 internal_set(a, b, m->current_frame->below, m->base_frame);
	 m->accum = b;
     }
}



void is_fn(machine *m)
{
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     
     if (equal(a, b)) {
	  m->accum = symbol(T);
     } else {
	  m->accum = nil();
     }
}


void oif_fn(machine *m)
{
     operation* newop = new(operation);
     newop->type = CLOSURE_OP;
     newop->next = m->current_frame->next;
     m->current_frame->next = newop;
     
     closure *test = get_arg(test, m);
     closure *then = get_arg(then, m);
     closure *elser = get_arg(elser, m);
     
     if (!nilp(test)) {
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
     if (acons->in->type != CONS_PAIR && !nilp(acons)){
       closure *sig = build_signal(cons(string(L"\n\nfinding the thousand things\nreflected within the tao\nis beyond me.\n\n\nerror: I attempted to retrieve the car from something that was not a cons pair: "), cons(acons, nil())), m);
       toss_signal(sig, m);
     } else {
	 // Streams should act like lazy stings...?
	 m->accum = car(acons);
     }
}


void cdr_fn(machine *m)
{
     closure *acons = get_arg(cons, m);
     if (acons->in->type != CONS_PAIR && acons->in->type != NIL){
       closure *sig = build_signal(cons(string(L"\n\nleaves rot on the warm dirt\nwhile the tree drinks rain\n he is not naked\n\n\nerror: I attempted to retrieve the cdr from something that is not a cons pair: "), cons(acons, nil())), m);
       toss_signal(sig, m);
     } else {
	 m->accum = cdr(acons);
     }
}

void atomp_fn(machine *m)
{
     closure *a = get_arg(a, m);
     if (a->in->type != CONS_PAIR){
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
    //print_stack(m->baseTT_frame);
}

void start_debug_fn(machine *m)
{
     DEBUG = 1;
}

void closing_of_fn(machine *m)
{
     closure *a = get_arg(a, m);
     m->accum = a->closing;
}

void leak_fn(machine *m)
{
     closure *sym = get_arg(sym, m);
     if (sym->in->type != SYMBOL && !nilp(sym)){
       closure *sig = build_signal(cons(string(L"\n\nHow can I forget\nthe juice of the orange\nthat is still dripping from my chin\n\n\nerror: I attempted to leak something that isn't a symbol: "), cons(sym, nil())), m);
       //  printf("You leaked a non symbol, man \n");
	  //print_closure((closure *) 5);
       toss_signal(sig, m);
     } else {
          closure *clos = copy_closure(get_arg(closure, m));
	  closure *isit = assoc(sym, clos->closing);
	  if (!nilp(isit)){
                isit->in->cons->car = symbol(LEAKED);
	  } else {
	        clos->closing = cheap_acons(sym, symbol(LEAKED), clos->closing);
	  }
	  // TODO
	  m->accum = clos;
     }
}

void comma_fn(machine *m)
{     
  operation* newop = new(operation);
     closure *clos = get_arg(closure, m);
     newop->next = m->current_frame->next;
     newop->type = CLOSURE_OP;
     newop->closure = clos;
     m->current_frame->next = newop;
}

void callcc_fn(machine *m)
{    
     closure *fn = get_arg(fn, m);
     closure *ret = new(closure);
     ret->in = new(doubleref);
     ret->in->type = CONTINUATION;
     ret->type = DREF;
     ret->closing = nil();
     ret->in->info = nil();
     ret->in->mach = stack_copy(m);
     // (quote ((a) ret))
     closure *cont = cons(cons(make_arg(a),  nil()),
				cons(ret, nil()));
     cont->in->info = 
	 cons(cons(symbol(FUNCTION_NAME),		
		   cons(symbol(string_to_symbol_id(L"continuation")),   
			nil())), nil());			   
     cont = cons(cleari(), cons(cont, nil()));
     closure *call = cons(fn, cons(cont, nil()));

     operation* newop = new(operation);
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
     frame *fm = find_signal_handler(m->current_frame);
     if (fm == NULL) {
	  printf("No handler.\n");
	  m->current_frame = m->base_frame;
	  m->accum = sig; //cdr(car(cdr(sig)));
	  return;
     };
     m->current_frame = fm;
     closure *fn = m->current_frame->signal_handler;
     
     sig = cheap_cons(fn, cheap_cons(sig, nil()));

     operation* newop = new(operation);
     newop->type = CLOSURE_OP;
     newop->closure = sig;
     m->current_frame->next = newop;
}

closure * build_signal(closure *a, machine *m)
{
     closure *ret = new(closure);
     ret->in = new(doubleref);
     ret->in->type = CONTINUATION;
     ret->type = DREF;
     ret->in->mach = stack_copy(m);
     ret->closing = nil();
     ret->in->info = nil();
     // (quote ((a) ret))
     closure *cont = cons(cons(make_arg(a),  nil()),
			  cons(ret, nil()));
     cont->in->info = 	 
	 cons(cons(symbol(FUNCTION_NAME),		
		   cons(symbol(string_to_symbol_id(L"continuation")),   
			nil())), nil());			   

     cont = cheap_cons(cont, cons(a, nil()));
     cont = cons(cleari(), cons(cont, nil()));
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
     m->current_frame = find_signal_handler(m->current_frame);
     m->current_frame = m->current_frame->below; // to remove the current handler
     toss_signal(quote(a), m);
}

void add_handler(machine *m){     
     closure *handler = get_arg(handler, m);
     m->current_frame->signal_handler = handler;
     operation* newop = new(operation);
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
     m->accum = number(a->in->num + b->in->num);
}
void minus_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = number(a->in->num - b->in->num);
}
void multiply_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = number(a->in->num * b->in->num);
}
void divide_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     m->accum = number(a->in->num / b->in->num);
}
void greater_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     if (a->in->num > b->in->num)
       m->accum = symbol(T);
     else 
       m->accum = nil();
}
void less_fn(machine *m){     
     closure *a = get_arg(a, m);
     closure *b = get_arg(b, m);
     if (a->in->num < b->in->num)
       m->accum = symbol(T);
     else 
       m->accum = nil();
}

void id_fn(machine *m){
     m->accum = get_arg(a, m);
};


void set_info_fn(machine *m){
    closure *a = get_arg(a, m);
    closure *info = get_arg(info, m);
    a->in->info = info;
    m->accum = a;
}

void get_info_fn(machine *m)
{
    closure *a = get_arg(a, m);
    m->accum = a->in->info;
}

void whitespacep_fn(machine *m)
{
    closure *a = get_arg(char, m);
    if(iswspace(a->in->character)){
	m->accum = symbol(T);
    } else {
	m->accum = nil();
    }
}

void eof_p_fn(machine *m)
{
    closure *c = get_arg(char, m);
    if(c->in->character == WEOF || c->in->character == EOF){
	m->accum = symbol(T);
    } else {
	m->accum = nil();
    }
}

void read_file_fn(machine *m)
{
    // This is stupid and will be until glibc 2.2
    // For now we have to convert files from wchar to char.
    closure *a = get_arg(filename, m);
    closure *handle = new(closure);
    handle->in = new(doubleref);
    handle->in->type = C_OBJECT;
    handle->type = DREF;
    handle->in->info = cons(cons(symbol(string_to_symbol_id(L"type")), cons(symbol(string_to_symbol_id(L"stream")), nil())), nil());
    handle->closing = nil();
    wchar_t *fn = string_to_c_MALLOC(a);
    char *rfn = calloc(wcslen(fn), sizeof(char));
    wcstombs(rfn, fn, wcslen(fn));
    handle->in->obj = (void *)fopen(rfn, "r");
    if(handle->in->obj == NULL){
	closure *sig = build_signal(cons(string(L"\n\nI organized the leaves by color\ngreen, red, gold, brown\nspread out for miles beneath the bare trees\nbut after I placed the last one\nthere was still a bare spot of dirt.\n\n\nerror: I attempted to open a file and failed: "), cons(a, nil())), m);
	toss_signal(sig, m);
    }
    free(fn);
    free(rfn);
    m->accum = cons(character(fgetwc(handle->in->obj)), cons(handle, nil()));
}

void close_file_fn(machine *m)
{
    closure *handle = get_arg(handle, m);
    fclose((FILE *)handle->in->obj);
    m->accum = nil();
}

void read_char_fn(machine *m)
{
    closure *handle = get_arg(handle, m);
    wchar_t c = fgetwc((FILE *)handle->in->obj);
    m->accum = character(c);
}

void set_car_fn(machine *m)
{
    closure *pair = get_arg(cons, m);
    closure *val = get_arg(value, m);
    pair->in->cons->car = val;
    m->accum = pair;
}

void set_cdr_fn(machine *m)
{
    closure *pair = get_arg(cons, m);
    closure *val = get_arg(value, m);
    pair->in->cons->cdr = val;
    m->accum = pair;
}

void string_to_symbol_fn(machine *m){
    closure *string = get_arg(string, m);
    m->accum = string_to_symbol(string);
}

void symbol_to_string_fn(machine *m){
    closure *sym = get_arg(sym, m);
    m->accum = string(symbol_id_to_string(sym->in->symbol_id));
}

void string_to_number_fn(machine *m){
    closure* str = get_arg(string, m);
    wchar_t *mstr = string_to_c_MALLOC(str);
    char *rstr = calloc(wcslen(mstr), sizeof(char));
    wcstombs(rstr, mstr, wcslen(mstr));

    m->accum = number(atoi(rstr));
    free(mstr);
}

void character_p_fn(machine *m){
    closure *achar = get_arg(character, m);
    if(achar->in->type == CHARACTER){
	m->accum = achar;
    } else {
	m->accum = nil();
    }
}
//------------------------------- FFI --------------------------------//


/* char *to_c_string(closure *str); */

/* closure *to_eight_string(char *str); */

/* int fix_to_int(closure *num); */

/* closure *int_to_fix(int num); */

/* void load_lib_fn(machine *m){ */
/*      closure *name = get_arg(name, m); */
/*      char *rname = to_c_string(name); */
/*      void *lib = dlopen(rname,RTLD_NOW|RTLD_GLOBAL); */
/*      closure *ret = new(closure); */
/*      ret->type = C_OBJECT; */
/*      ret->value = lib; */
/*      m->accum = ret; */
/* } */

/* void get_lib_object_fn(machine *m){ */
/*      closure *lib = get_arg(lib, m); */
/*      closure *name = get_arg(name, m); */
/*      char *rname = to_c_string(name); */
/*      void *obj = dlsym(rname, (lib->value)); */
/*      closure *ret = new(closure); */
/*      ret->type = C_OBJECT; */
/*      ret->value = &obj; */
/*      m->accum = ret; */
/* } */

/* void call_c_fn(machine *m){ */
/*      closure *fn = get_arg(fn, m); */
/*      closure *args = get_arg(args, m); */
/*      int len = length(args); */
/*      void * (*pooer)() = (fn->value); */
/*      void *rret; */
/*      switch (len) { */
/*      case 0: rret = &(pooer()); */
/*      } */
/*      closure *ret = new(closure); */
/*      ret->type = C_OBJECT; */
/*      ret->value = rret; */
/*      m->accum = ret; */
/* } */

//--------------------------------------------------------------------//


void new_basic_commands(machine *m)
{	  
    // TODO This is still more complicated than it should be.
     closure *sym;
     closure *val;

     intern_fn(is, &is_fn, list(2, make_arg(a), make_arg(b)), m);
  
     intern_fn(set!, &set_fn, list(2, 
				   quote(make_arg(a)), 
				   make_arg(b)), m);
     intern_fn(oif, &oif_fn, list(3, 
				  make_arg(test),
				  quote(make_arg(then)),//arg
				  quote(make_arg(elser))), m); // nil

     intern_fn(cons, &cons_fn, cons(make_arg(car), 
				cons(make_arg(cdr), nil())), m);

     intern_fn(car, &car_fn, cons(make_arg(cons), nil()), m);

     intern_fn(cdr, &cdr_fn, cons(make_arg(cons), nil()), m);

     intern_fn(atom-p, &atomp_fn, cons(make_arg(a), nil()), m);

     intern_fn(print, &print_fn, cons(make_arg(a), nil()), m);

     intern_fn(leak, &leak_fn, cons(make_arg(sym), 
				cons(make_arg(closure), nil())), m);

     intern_fn(closing-of, &closing_of_fn, cons(make_arg(a), nil()), m);
        
     intern_fn(comma, &comma_fn, cons(make_arg(closure),  nil()), m);
   
     intern_fn(prmachine, &prmachine_fn, nil(), m);

     intern_fn(start-debug, &start_debug_fn, nil(), m);

     intern_fn(call/cc, &callcc_fn, 
	       cons(quote(make_arg(fn)), nil()), m);

     intern_fn(signal, &signal_fn, cons(make_arg(sig),  nil()), m);

     intern_fn(handle-signals, 
	       &add_handler, 
	       cons(quote(make_arg(handler)),  
		    cons(quote(make_arg(body)), nil())), m);

     intern_fn(unhandle-signal, &unhandle_signal, 
	       cons(make_arg(sig),  nil()), m);

     intern_fn(base-signal-handler, &base_handler, 
	       cons(quote(make_arg(handler)),  nil()), m);
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

     intern_fn(set-info, &set_info_fn, cons(make_arg(a), 
					    cons(make_arg(info), nil())), m);

     intern_fn(get-info, &get_info_fn, cons(make_arg(a), nil()), m);

     intern_fn(whitespace-p, &whitespacep_fn, cons(make_arg(char), nil()), m);

     intern_fn(eof-p, &eof_p_fn, cons(make_arg(char), nil()), m);

     intern_fn(read-file, &read_file_fn, cons(make_arg(filename), nil()), m);

     intern_fn(close-file, &close_file_fn, cons(make_arg(handle), nil()), m);

     intern_fn(read-character, &read_char_fn, cons(make_arg(handle), nil()), m);

     intern_fn(set-car, &set_car_fn, cons(make_arg(cons), 
					    cons(make_arg(value), nil())), m);

     intern_fn(set-cdr, &set_cdr_fn, cons(make_arg(cons), 
					    cons(make_arg(value), nil())), m);

     intern_fn(string-to-symbol, &string_to_symbol_fn, cons(make_arg(string), nil()), m);

     intern_fn(symbol-to-string, &symbol_to_string_fn, cons(make_arg(sym), nil()), m);

     intern_fn(string-to-number, &string_to_number_fn, cons(make_arg(string), nil()), m);

     intern_fn(character-p, &character_p_fn, cons(make_arg(character), nil()), m);


}    

#endif
 

