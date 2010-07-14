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

#define get_arg(sym, m) car(looker_up(symbol(string_to_symbol_id(L""#sym)), \
				      m->current_frame, m->base_frame))     	
	
void internify(wchar_t *fn_name, void *fn_pointer, closure *lambda_list, machine *m, wchar_t *help_string)
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
      list(2, 
	   list(2, symbol(FUNCTION_NAME),		
		symbol(string_to_symbol_id(fn_name))),
	   list(2, symbol(HELP),
		string(help_string)));
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
	      list(2, make_arg(a), make_arg(b)), m,
	      L"The function (is) tests for simple equality; it takes two arguments\nand returns t when those arguments are equal, or () when they are\nnot. Two things are equal if they are the same symbol, the same\nobject, or equal numbers. Lists with equal elements made at different\ntimes are NOT equal:\n\n(is 3 3)\nt\n(set! a '(1 2 3))\n(is a a)\nt\n(is a '(1 2 3))\n()\n\n");
 
    internify(L"'", 
	      &quote_fn, 
	      list(1, make_arg(x)), m, 
	      L"The function ' takes one argument and returns that argument UNMODIFIED\nand UNEVALUATED, but with an associated closure --- so that if the\nreturn value is evaluated at any time in the future, it returns what\nit would have returned, had it been evaluated at the time of '-ing.");
    
    internify(L"oif", &oif_fn, 
	      list(3, 
		   make_arg(test),
		   quote(make_arg(then)),
		   quote(make_arg(elser))), 
	      m,
	      L"oif is a simple conditional. It takes a test and two statements:\n\n(if (is 3 3)\n    (print \"Yes!\")\n    (print \"no...\"))\nYes!\n\nIf the test, when evaluated, returns (), then the second statement is\nevaluated --- here, that would be (print \"no...\"). If, as is the\ncase here, test returns anything BUT (), then the first statement is\nevaluated: (print \"Yes!\")"); 
    
    internify(L"cons", &cons_fn, 
	      list(2, make_arg(car), make_arg(cdr)), 
	      m, L"cons takes two arguments and builds a cons-pair from them. Please do\nnot use cons; use join whenever possible. Please ALWAYS ensure that\nthe result of using cons is a 'proper list' --- that is, one that ends\nwith a (). You can tell if your list is a proper list by calling\n(print list); if a . appears anywhere in your structure, you've used\ncons innapropriately. Jerk.");
    
    internify(L"car", &car_fn, list(1, make_arg(cons)), m, L"The function car is a low-level way of retrieving the first element of\na list. Please use (first) instead. Car should be used only when speed\nis of the essence --- when I can also reccomend using something other\nthan Eight.");
    
    internify(L"cdr", &cdr_fn, list(1, make_arg(cons)), m, L"The function car is a low-level way of retrieving all but the first element of\na list. Please use (rest) instead. cdr should be used only when you wish to be speedy. \nYou're reading this. So you don't.");
    
    internify(L",", &comma_fn, list(1, make_arg(closure)), m, L", is a function that takes one argument and evaluates it.\n\n(, '(plus 2 3))\n5\n\nThink of it as the inverse of '. See (help ').");
    
    internify(L"call/cc", &callcc_fn, 
	      list(1, quote(make_arg(fn))), 
	      m, L"Woah boy. Call with current continuation is hard. So here's a\nwikipedia article instead:\nhttp://en.wikipedia.org/wiki/Call-with-current-continuation");
    
    
    
    
    internify(L"atom-p", &atomp_fn, list(1, make_arg(a)), m,
	      L"atom-p takes one argument and returns t if that argument is not a list, and () otherwise.");
    
    internify(L"leak", &leak_fn, 
	      list(2, make_arg(sym), make_arg(closure)), 
	      m, L"BANANA BANANA BANANA leak is hard to explain. Please yell at me so that I write copy on this. \n\ndiiq is too silly\nto write documentation\npotato salad.\n\n");
    
    
    
    

    internify(L"set!", &set_fn, 
	      list(2, quote(make_arg(a)), make_arg(b)), 
	      m, L"set! takes two arguments; the first is a symbol, and is NOT evaluated;\nthe second argument is evaluated. The symbol is bound to that value.\n\n(set! a 5)\n5\na\n5\n");
    
    internify(L"set-car", &set_car_fn, 
	      list(2, make_arg(cons), make_arg(value)), 
	      m, L"set-car takes two arguments; a list, and a statement. The first\nelement of the list is set to the value of the statement.");
    
    internify(L"set-cdr", &set_cdr_fn, 
	      list(2, make_arg(cons), make_arg(value)), 
	      m, L"set-car takes two arguments; a list, and a statement. All but the first element of the list is set to the value of the statement.");
    
    
    
    
    
    internify(L"signal", &signal_fn, list(1, make_arg(sig)), m, L"Signal takes one argument. It builds a signal which is a list; the first element of the list is a continuation [see (help call/cc)] and the second element is whatever you passed to (signal). That whole package is then passed to the nearest signal-handler. See (help handle-signals), (help unhandle-signal), and (help base-signal-handler).");
     
    internify(L"handle-signals", &add_handler, 
	      list(2,
		   quote(make_arg(handler)),  
		   //		   symbol(ELIPSIS), 
		   quote(make_arg(body))), 
	      m, L"handle-signals takes two arguments. The first argument is a unary function; the second is code. If the code throws a signal, that signal will be handed to the function. See (help signal), (help unhandle-signal), and (help base-signal-handler).");
     
    internify(L"unhandle-signal", &unhandle_signal, 
	      list(1, make_arg(sig)), 
	      m, L"See (help signal), (help handle-signals), and (help base-signal-handler).");
     
    internify(L"base-signal-handler", &base_handler, 
	      list(1, quote(make_arg(handler))), 
	      m, L"See (help signal), (help handle-signals), and (help unhandle-signal).");





    internify(L"closing-of", &closing_of_fn, list(1, make_arg(a)), m, L"");

    internify(L"set-info", &set_info_fn, 
	      list(2, make_arg(a), make_arg(info)), 
	      m, L"");

    internify(L"get-info", &get_info_fn, list(1, make_arg(a)), m, L"");




    internify(L"print", &print_fn, list(2, symbol(ELIPSIS), make_arg(a)), m, L"");
     
    internify(L"prmachine", &prmachine_fn, nil(), m, L"");
     
    internify(L"start-debug", &start_debug_fn, nil(), m, L"");

    internify(L"stack-trace", 
	      &stack_trace_fn, 
	      list(1, make_arg(continuation)), m, L"");

     
    internify(L"read-file", &read_file_fn, list(1, make_arg(filename)), m, L"");

    internify(L"close-file", &close_file_fn, list(1, make_arg(handle)), m, L"");

    internify(L"read-character", &read_char_fn, list(1, make_arg(handle)), m, L"");




    internify(L"whitespace-p", &whitespacep_fn, list(1, make_arg(char)), m, L"");

    internify(L"eof-p", &eof_p_fn, list(1, make_arg(char)), m, L"");

    internify(L"string-to-symbol", &string_to_symbol_fn, 
	      list(1, make_arg(string)), 
	      m, L"");

    internify(L"symbol-to-string", &symbol_to_string_fn, 
	      list(1, make_arg(sym)), 
	      m, L"");
     
    internify(L"string-to-number", &string_to_number_fn, 
	      list(1, make_arg(string)), 
	      m, L"");

    internify(L"character-p", &character_p_fn, 
	      list(1, make_arg(character)), 
	      m, L"");




     internify(L"plus", &plus_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     internify(L"minus", &minus_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     internify(L"multiply", &multiply_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     internify(L"divide", &divide_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     internify(L">", &greater_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     internify(L"<", &less_fn, list(2, make_arg(a), make_arg(b)), m, L"");

     /*
     internify(L"load-library", &load_library_fn, list(1, make_arg(name)), m, L"");

     internify(L"load-library-object", 
	       &load_library_object_fn, 
	       list(2, make_arg(name), make_arg(library)), m, L"");

     //     internify(L"call-library-function", 
     //	       &call_library_function_fn, 
     //	       list(3, make_arg(fn), symbol(ELIPSIS), make_arg(args)), m, L"");
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
    //    if (!nilp(f->function)){
    if (f->next){
	return list(4, f->function, f->rib, f->scope, f->next->closure);
    }	else {
	return list(4, f->function, f->rib, f->scope, nil());
    }	
	//} 
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
	  print_closure(sig);
	  print_info(m);
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






 

