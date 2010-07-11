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


// These are some clever macros to make interning function slightly less
// painful. Any time we have to express lisp in c, it's going to be 
// unpleasant. These make it readable, at least.

#define make_arg(sym) symbol(string_to_symbol_id(L""#sym))

	
void internify(wchar_t *fn_name, void *fn_pointer, closure *lambda_list, machine *m)
{
    closure *sym; closure *val;
    sym = new(closure);			
    sym->in = new(doubleref);		
    sym->in->type = BUILTIN;     	
    sym->type = DREF;					
    sym->in->builtin_fn = fn_pointer;			
    sym->closing = nil();				
    sym->in->info = nil();				
    val = list(2, lambda_list, sym);			
    val->in->info =					
      list(1, list(2, symbol(FUNCTION_NAME),		
		   symbol(string_to_symbol_id(fn_name))));
    internal_set(symbol(string_to_symbol_id(fn_name)),
		 val,				
		 m->current_frame,		
		 m->base_frame);		
}


void intern_builtin_functions(machine *m)
{	  
    // This function nterns all of the builtin functions: oif, is, +, &c.
 
    // TODO This is still more complicated than it should be.

    internify(L"is", &is_fn, 
	      list(2, make_arg(a), make_arg(b)), m);
 
    internify(L"'", 
	      &quote_fn, 
	      list(1, make_arg(x)), m);
    
    internify(L"oif", &oif_fn, 
	      list(3, 
		   make_arg(test),
		   quote(make_arg(then)),
		   quote(make_arg(elser))), 
	      m); 
    
    internify(L"cons", &cons_fn, 
	      list(2, make_arg(car), make_arg(cdr)), 
	       m);
    
    internify(L"car", &car_fn, list(1, make_arg(cons)), m);
    
    internify(L"cdr", &cdr_fn, list(1, make_arg(cons)), m);
    
    internify(L",", &comma_fn, list(1, make_arg(closure)), m);
    
    internify(L"call/cc", &callcc_fn, 
	      list(1, quote(make_arg(fn))), 
	      m);
    
    
    
    
    internify(L"atom-p", &atomp_fn, list(1, make_arg(a)), m);
    
    internify(L"leak", &leak_fn, 
	      list(2, make_arg(sym), make_arg(closure)), 
	      m);
    
    
    
    

    internify(L"set!", &set_fn, 
	      list(2, quote(make_arg(a)), make_arg(b)), 
	      m);
    
    internify(L"set-car", &set_car_fn, 
	      list(2, make_arg(cons), make_arg(value)), 
	      m);
    
    internify(L"set-cdr", &set_cdr_fn, 
	      list(2, make_arg(cons), make_arg(value)), 
	      m);
    
    
    
    
    
    internify(L"signal", &signal_fn, list(1, make_arg(sig)), m);
     
    internify(L"handle-signals", &add_handler, 
	      list(2,
		   quote(make_arg(handler)),  
		   quote(make_arg(body))), 
	      m);
     
    internify(L"unhandle-signal", &unhandle_signal, 
	      list(1, make_arg(sig)), 
	      m);
     
    internify(L"base-signal-handler", &base_handler, 
	      list(1, quote(make_arg(handler))), 
	      m);





    internify(L"closing-of", &closing_of_fn, list(1, make_arg(a)), m);

    internify(L"set-info", &set_info_fn, 
	      list(2, make_arg(a), make_arg(info)), 
	      m);

    internify(L"get-info", &get_info_fn, list(1, make_arg(a)), m);




    internify(L"print", &print_fn, list(2, symbol(ELIPSIS), make_arg(a)), m);
     
    internify(L"prmachine", &prmachine_fn, nil(), m);
     
    internify(L"start-debug", &start_debug_fn, nil(), m);

    internify(L"stack-trace", 
	      &stack_trace_fn, 
	      list(1, make_arg(continuation)), m);

     
    internify(L"read-file", &read_file_fn, list(1, make_arg(filename)), m);

    internify(L"close-file", &close_file_fn, list(1, make_arg(handle)), m);

    internify(L"read-character", &read_char_fn, list(1, make_arg(handle)), m);




    internify(L"whitespace-p", &whitespacep_fn, list(1, make_arg(char)), m);

    internify(L"eof-p", &eof_p_fn, list(1, make_arg(char)), m);

    internify(L"string-to-symbol", &string_to_symbol_fn, 
	      list(1, make_arg(string)), 
	      m);

    internify(L"symbol-to-string", &symbol_to_string_fn, 
	      list(1, make_arg(sym)), 
	      m);
     
    internify(L"string-to-number", &string_to_number_fn, 
	      list(1, make_arg(string)), 
	      m);

    internify(L"character-p", &character_p_fn, 
	      list(1, make_arg(character)), 
	      m);




     internify(L"plus", &plus_fn, list(2, make_arg(a), make_arg(b)), m);

     internify(L"minus", &minus_fn, list(2, make_arg(a), make_arg(b)), m);

     internify(L"multiply", &multiply_fn, list(2, make_arg(a), make_arg(b)), m);

     internify(L"divide", &divide_fn, list(2, make_arg(a), make_arg(b)), m);

     internify(L">", &greater_fn, list(2, make_arg(a), make_arg(b)), m);

     internify(L"<", &less_fn, list(2, make_arg(a), make_arg(b)), m);

     /*
     internify(L"load-library", &load_library_fn, list(1, make_arg(name)), m);

     internify(L"load-library-object", 
	       &load_library_object_fn, 
	       list(2, make_arg(name), make_arg(library)), m);

     //     internify(L"call-library-function", 
     //	       &call_library_function_fn, 
     //	       list(3, make_arg(fn), symbol(ELIPSIS), make_arg(args)), m);
     */
}    


// Down here are all the actual functions that are referred to up in
// intern_builtin_functions.


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

void quote_fn(machine *m)
{
    closure *x = get_arg(x, m);
    m->accum = enclose(x, m->current_frame, m->base_frame);
    // TODO sanity check, is lonely cdr?
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
     while(!nilp(a)){
       if(stringp(car(a))){
         wchar_t *str = string_to_c_MALLOC(car(a));
         printf("%ls", str);
       } else
         print_closure(car(a));
       a = cdr(a);
     }
     fflush(stdout);
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

//--------- This is all for stack trace --------------//
closure* build_stack_trace(frame *f);
closure *frame_trace(frame *f);

void stack_trace_fn(machine *m)
{
    closure *a = get_arg(continuation, m);
    m->accum = build_stack_trace(second(a)->in->mach->current_frame);
}

closure *build_stack_trace(frame *f)
{
    if (f->below != NULL)
      return cons(frame_trace(f),
                  build_stack_trace(f->below));
    return nil();
}

closure *frame_trace(frame *f)
{
    if (!nilp(f->function)){
	return list(2, f->function, f->rib);
    } 
    return nil();
}

//-------------------- End --------------------------//

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
       toss_signal(sig, m);
     } else {
          closure *clos = copy_closure(get_arg(closure, m));
	  closure *isit = assoc(sym, clos->closing);
	  if (!nilp(isit)){
                isit->in->cons->car = symbol(LEAKED);
	  } else {
	        clos->closing = cheap_acons(sym, symbol(LEAKED), clos->closing);
	  }
	  m->accum = clos;
     }
}

void comma_fn(machine *m)
{     
    operation* newop = new(operation);
    closure *clos = get_arg(closure, m);
    //    m->current_frame = m->current_frame->below;
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
     closure *cont = list(2, list(1, make_arg(a)),
			  ret);
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
    fclose((FILE *)(last(handle)->in->obj));
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

/* #include <dlfcn.h> */

/* void load_library_fn(machine *m) */
/* { */
/*      closure *name = get_arg(name, m); */
/*      wchar_t *rname = string_to_c_MALLOC(name); */
/*      char *rrname = calloc(wcslen(rname), sizeof(char)); */
/*      wcstombs(rrname, rname, wcslen(rname)); */
/*      void *lib = dlopen(rrname,RTLD_LAZY); */
/*      if (lib == NULL) { */
/* 	 printf("Well, shit.\n"); */
/* 	 fputs (dlerror(), stderr); */
/*      } */
/*      free(rname); */
/*      free(rrname); */
/*      closure *ret = new(closure); */
/*      ret->type = DREF; */
/*      ret->in = new(doubleref); */
/*      ret->in->type = C_OBJECT; */
/*      ret->in->c_object = lib; */

/*      m->accum = ret; */
/* } */

/* void load_library_object_fn(machine *m) */
/* { */
/*      closure *lib = get_arg(library, m); */
/*      closure *name = get_arg(name, m); */

/*      wchar_t *rname = string_to_c_MALLOC(name); */
/*      char *rrname = calloc(wcslen(rname), sizeof(char)); */
/*      wcstombs(rrname, rname, wcslen(rname)); */

/*      void *obj = dlsym((lib->in->c_object), rrname); */
/*      if (obj == NULL) { */
/* 	 printf("Well, shit.\n"); */
/* 	 fputs (dlerror(), stderr); */
/*      } */
/*      free(rname); */
/*      free(rrname); */

/*      closure *ret = new(closure); */
/*      ret->type = DREF; */
/*      ret->in = new(doubleref); */
/*      ret->in->type = C_OBJECT; */
/*      ret->in->c_object = obj; */
/*      m->accum = ret; */
/* } */

/* void call_library_function_fn(machine *m) */
/* { */
/*     closure *fn = get_arg(fn, m); */
/*     closure *args = get_arg(args, m); */
/*     void * (*pooer)(machine *m, closure* args) = (fn->in->c_object); */
/*     pooer(m, args); */
/* } */

//--------------------------------------------------------------------//



#endif






 

