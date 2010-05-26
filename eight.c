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
#ifndef EIGHT_MAIN
#define EIGHT_MAIN

#include "eight.h"
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "symbols.c"

machine *init_8VM()

{
  //  test_memory();
  // error(32623653, 432747235, "got there");
     machine *m =  init_memory();
     frame *bframe = new_frame(NULL);
     m->current_frame = bframe;
     m->base_frame = bframe;

     insert_symbol(L"'", QUOTE);
     insert_symbol(L"*", ASTERIX);
     insert_symbol(L"@", ATPEND);
     insert_symbol(L",", COMMA);
     insert_symbol(L"...", ELIPSIS);
     insert_symbol(L"t", T);
     insert_symbol(L"function-name", FUNCTION_NAME);

     internal_set(symbol(T), symbol(T), bframe, bframe);
     new_basic_commands(m);
     //m = collect();

     return m;
}	

closure *cleari()
{
     closure *cleari = symbol(CLEAR); 
     cleari->in->type = INTERNAL;
     return cleari;
}

closure *e_argument() 
{
     closure *e_argument = symbol(E_ARGUMENT); 
     e_argument->in->type = INTERNAL;
     return e_argument;
}


int nilp(closure *x)
{
     if(x->in->type == NIL) return 1;
     return 0;
}

closure *symbol(symbol_id id)
{
     // builds a symbol-closure from an id.
     closure *sym = new(closure);
     sym->in = new(doubleref);
     sym->in->type = SYMBOL;
     sym->type = DREF;
     sym->in->symbol_id = id;
     sym->closing = nil();
     sym->in->info = nil();
     return sym; 
}

int leakedp(closure *x)
{
     closure *y = cheap_car(x);
     if(y->in->type == SYMBOL && y->in->symbol_id == LEAKED) return 1;
  return 0;
}

closure *number(int num)
{
  closure *ret = new(closure);
  ret->in = new(doubleref);
  ret->in->type = NUMBER;
  ret->type = DREF;
  ret->closing = nil();
  ret->in->info = nil();
  ret->in->num = num;
  return ret;
}

closure *quote(closure *x)
{
      closure *q = symbol(QUOTE);
      return cheap_cons(q, cons(x, nil()));
};

closure *e_arg()
{
     closure *a = symbol(E_ARGUMENT);
     a->in->type = INTERNAL;
     return a;
}

int equal(closure *a, closure *b)
{
     if ((a==b) ||
	 (nilp(a) && nilp(b)) ||
	 ((a->in->type == b->in->type) && (a->in->obj == b->in->obj)))
	  return 1;
     return 0;
}


closure *cheap_cons(closure *car, closure *cdr)
{
     closure *pair = new(closure);
     pair->type = DREF;
     pair->in = new(doubleref);
     pair->in->type = CONS_PAIR;
     cons_cell *inner = new(cons_cell);
     inner->type = CONS_CELL;
 
     pair->in->cons = inner;
     inner->car = car;
     inner->cdr = cdr;
     pair->closing = nil();
     pair->in->info = nil();
     return pair;
}

closure *copy_closure(closure *x)
{
     closure *newx = new(closure);
     memcpy(newx, x, sizeof(closure));
     return newx;
}

closure *cons(closure *car, closure *cdr)
{
     closure *pair = new(closure);
     pair->in = new(doubleref);
     pair->in->type = CONS_PAIR;
     pair->in->info = nil();
     cons_cell *cpair = new(cons_cell);
     cpair->type = CONS_CELL;
     pair->type = DREF;
     pair->in->cons = cpair;
     if (nilp(cdr)) {
	  closure *newcar = copy_closure(car);
	  pair->closing = car->closing;
	  newcar->closing = nil();
	  pair->in->cons->car = newcar;
	  pair->in->cons->cdr = nil();
	  cdr->closing = nil();
     } else if (nilp(car)){
	  closure *newcdr = copy_closure(cdr);
	  pair->closing = cdr->closing;
	  newcdr->closing = nil();
	  pair->in->cons->cdr = newcdr;
	  pair->in->cons->car = nil();
	  car->closing = nil();
     } else {	
	  closure *newa = nil();
	  closure *newb = nil();
	  closure *newc = nil();
	  combine(car->closing, 
		  cdr->closing,
		  &newa, &newb, &newc);
	  combine(cdr->closing, 
		  nil(),
		  &newb, &newa, &newc);
	  closure *ncar = copy_closure(car);
	  closure *ncdr = copy_closure(cdr);
	  ncar->closing = newa;
	  ncdr->closing = newb;
	  pair->closing = newc;
	  pair->in->cons->car = ncar;
	  pair->in->cons->cdr = ncdr;
     }
     return pair;
};

void combine(closure *a, 
	     closure *b, 
	     closure **newa, 
	     closure **newb,
	     closure **ret)
{
     // So each closing should look like:
     // ([(sym val . ())]* . ())
     // So car(car( is sym;
     // car(cdr(car is val
     // cdr( is rest
     if (nilp(a)){ // halt condition
	  return;
     }
     closure *seen = assoc(cheap_car(cheap_car(a)), *ret); // look for symbol
                                               // in the new closure;
     if(nilp(seen)){ // if it isn't there, then check for conflicts...
                     // look for symbol in b...

	  closure *duplicate = assoc(cheap_car(cheap_car(a)), b);
	  if(nilp(duplicate) || 
	     equal(cheap_car(duplicate), cheap_car( cheap_cdr (cheap_car(a))))){ 
	       // No conflict? Then toss it in ret.
	       *ret = cheap_cons(cheap_car(a), *ret);
	  } else {
	       *newa = cheap_cons(cheap_car(a), *newa);
	       *newb = cheap_cons(cheap_cons(cheap_car(cheap_car(a)), duplicate), *newb);
	  }
     }
     combine(cheap_cdr(a), b, newa, newb, ret); 
}


closure *rectify_closing(closure *closed)
{
     closed->closing = rectify_closing_i(closed, closed->closing, nil());
     return closed;
}

closure *rectify_closing_i(closure *closed, closure *closing, closure *ret)
{
     if (closed->in->type == SYMBOL){
	  closure* value = assoc(closed, closing);
	  if (!nilp(value)){
	       ret = cheap_acons(closed,
				 value->in->cons->car, 
				 ret);
	  }
     } else if (closed->in->type == CONS_PAIR && !quotep(closed)){
	  ret = rectify_closing_i(closed->in->cons->car,
				  closing,
				  ret);
	  ret = rectify_closing_i(closed->in->cons->cdr, 
				  closing,
				  ret);
     }
     return ret;
}

closure *cheap_car(closure *x)
{
     if (nilp(x)) return x;
     return x->in->cons->car;
}

closure *cheap_cdr(closure *x)
{
     if (nilp(x)) return x;
     return x->in->cons->cdr;
}
	       
closure *car(closure *x)
{
     if (nilp(x)) return x;
     //if (x->type != CONS_PAIR) return NULL;
     if (nilp(x->closing)){
	 return copy_closure(x->in->cons->car);
     } else if (nilp(x->in->cons->car)){
	  return nil();
     } else {
	  closure *clo = copy_closure(x->in->cons->car);
	  clo->closing = cheap_append(clo->closing, x->closing);
     	  return rectify_closing(clo);
     } 
}

closure *cdr(closure *x)
{
     if (nilp(x)) return x;
     //if (x->type != CONS_PAIR) return NULL;
     if (nilp(x->closing)){
	  return copy_closure(x->in->cons->cdr);
     } else if (nilp(x->in->cons->cdr)){
	  return nil();
     } else {
	  closure *clo = copy_closure(x->in->cons->cdr);
	  clo->closing = cheap_append(clo->closing, x->closing);
     	  return rectify_closing(clo);
     } 
}


closure *append(closure *a, closure *b)
{
     // TODO sanity test a & b for listness.
     if (nilp(a)){
	  return b;
     } else {
	  return cons(car(a), append(cdr(a), b));
     }
};

closure *cheap_append(closure *a, closure *b)
{
     // TODO sanity test a & b for listness.
     if (nilp(a)){
	  return b;
     } else {
	  return cheap_cons(cheap_car(a), cheap_append(cheap_cdr(a), b));
     }
};

closure *second(closure* list)
{
     return car(cdr(list));
}

closure *assoc(closure *sym, closure* closing)
{
     if (nilp(closing)) {
	  return nil();
     }
     if (equal(sym, cheap_car(cheap_car(closing)))) {
	  return cheap_cdr(cheap_car(closing));
     }
     return assoc(sym, cheap_cdr(closing));
};

closure *cheap_acons(closure *sym, closure *val, closure *closing)
{
     closure *ret =  cheap_cons(
	  cheap_cons(sym, cheap_cons(val, nil())), 
	  closing);
     return ret;
};

int length(closure *xs)
{
  int i = 0;
  while(!nilp(xs)){
    i++;
    xs = cdr(xs);
  }
  return i;
}

closure *looker_up(closure *sym, frame *aframe)
{
     // note that the value itself is the car of
     // what is returned; by setting the car of
     // what is returned, you can alter the value
     // destructively.
     closure *local = assoc(sym, sym->closing);
     if (nilp(local) || leakedp(local)) 
       return looker_up_internal(sym, aframe);
     return local;
};

closure *looker_up_internal(closure *sym, frame *aframe)
{
     closure* local =  assoc(sym, aframe->scope);
     if (nilp(local) || leakedp(local)) {
       if (aframe->below == NULL)
	    return nil();
       return looker_up_internal(sym, aframe->below);
     }
     return local;
};

void internal_set(closure *sym,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame)
{
     closure *old = looker_up(sym, aframe);
     if (nilp(old)){
	  base_frame->scope = cheap_acons(sym,
					  value,
					  base_frame->scope);
     } else {
	 memcpy(old->in->cons->car, value, sizeof(closure));
     };
}

int commap(closure *arg)
{
     if ((arg->in->type == CONS_PAIR) &&
	 (cheap_car(arg)->in->type == SYMBOL) &&
	 (cheap_car(arg)->in->symbol_id == COMMA)) return 1;
     return 0;
}

int asterixp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->in->type == CONS_PAIR) &&
	 (cheap_car(arg)->in->type == SYMBOL) &&
	 (cheap_car(arg)->in->symbol_id == ASTERIX)) return 1;
     return 0;
};
int atpendp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->in->type == CONS_PAIR) &&
	 (cheap_car(arg)->in->type == SYMBOL) &&
	 (cheap_car(arg)->in->symbol_id == ATPEND)) return 1;
     return 0;
};
int quotep(closure *sym)
{
     // returns true if argument == *x
     if ((sym->in->type == CONS_PAIR) &&
	 (cheap_car(sym)->in->type == SYMBOL) &&
	 (cheap_car(sym)->in->symbol_id == QUOTE)) return 1;
     return 0;
};
int elipsisp(closure *sym)
{
     //returns true if sym == ...
     if ((sym->in->type == SYMBOL) &&
	 (sym->in->symbol_id == ELIPSIS)) return 1;
     return 0;
}

int optional_argp(closure *sym)
{
     if ((sym->in->type == CONS_PAIR) &&
	 (cheap_car(sym)->in->type == SYMBOL) &&
	 (cheap_car(sym)->in->symbol_id != QUOTE))  return 1;
     return 0;
};

int e_argp(closure *sym)
{
     if (((sym->in->type == INTERNAL) && 
	  (sym->in->symbol_id == E_ARGUMENT)) ||
	 ((sym->in->type == CONS_PAIR) &&
	  ((second(sym)->in->type == INTERNAL) && 
	   (second(sym)->in->symbol_id == E_ARGUMENT)))) return 1;
     return 0;
}


operation *build_argument_chain(closure *lambda_list,
				closure *arg_list,
				operation *current)
{
     // builds the operations necessary to evaluate the arguments of a fn.
     if (nilp(lambda_list)) {
	  if (!nilp(arg_list)) {
	      return (operation *)-1;
	  }
	  // If we're out of lambda list, we're done!
	  return current;
     } else if (asterixp(cheap_car(arg_list))){
	  // If the first argument is a *arg, then we need to grab
	  // the actual argument, which is the ((asterix HERE) ...)
	  // then we build a continue-apply structure so that we can 
	  // pause and evaluate the argument.
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))

	  operation *tnext = new(operation);
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = new(operation);
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = CONTINUE_APPLY;
	  bnext->closure = cheap_cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (atpendp(car(arg_list))){
     // It's a similar story with atpend as asterix;
     // a different machine flag, but that's all.
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))
	  operation *tnext = new(operation);
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = new(operation);
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = ATPEND_APPLY;
	  bnext->closure = cheap_cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (elipsisp(car(lambda_list))){
	  // The trick here is that you have to signal yourself
	  // that you're elipsising, even THROUGH a continue_apply
	  closure *argm;
	  if (quotep(second(lambda_list))){
	       // if the next arg is quoted, keep the quote.
	       argm = cons(quote(e_argument()), nil());
	  } else {
	       argm = cons(e_argument(), nil());
	  }
	  if (nilp(arg_list)){
	       return build_argument_chain(nil(),
					   arg_list,
					   make_arg(second(lambda_list),
						    nil(),
						    current));
	  } else {
	       return build_argument_chain(argm,
					   arg_list,
					   make_arg(second(lambda_list),
						    nil(),
						    current));
	  }	
     } else if (optional_argp(car(lambda_list))){
	  // if there is a parameter given, remove the 'optional' and rec.
	  // if there isn't, push the default form on to arg_list, and rec.
	  if (!nilp(arg_list)) {
	       return build_argument_chain(cons(car(car(lambda_list)),
						cdr(lambda_list)),
					   arg_list,
					   current);
	  } else {
	       return build_argument_chain(cons(car(car(lambda_list)),
						cdr(lambda_list)),
					   cons(second(car(lambda_list)),
						nil()),
					   current);
	  }
     } else {
	  // e_argument makes this kinda a mess, but not too bad:
	  // just check for e-arg and if it's present, DON'T shrink the
	  // lambda list until there are no more arguments in arg_list.
	  if (e_argp(car(lambda_list)) && !nilp(cdr(arg_list))){
	       return build_argument_chain(lambda_list,
					   cdr(arg_list),
					   make_arg(car(lambda_list),
						    car(arg_list),
						    current));
	  } else {
	       return build_argument_chain(cdr(lambda_list),
					   cdr(arg_list),
					   make_arg(car(lambda_list),
						    car(arg_list),
						    current));
	  }
     }
}

operation *make_arg(closure *sym, closure *val, operation *current)
{
     operation* arg  = new(operation);
     operation* arg2 = new(operation);
     current->next = arg;
     current = arg;
     current->type = CLOSURE_OP;
     current->next = arg2;
     // it's possible these can be simplified to conses not lists.
     if (quotep(sym)){
	  if (commap(val)){
	       //(closure_op (second val))->(machine_op, argument (second sym))
	       current->closure = second(val);
	       current = arg2;
	       current->type = MACHINE_FLAG;
	       current->flag = ARGUMENT;
	       current->closure = second(sym);
	  } else {
	       //(closure_op (quote val))->(machine_op, argument (second sym)))
	       current->closure = quote(val);
	       current = arg2;
	       current->type = MACHINE_FLAG;
	       current->flag = ARGUMENT;
	       current->closure = second(sym);
	  }
     } else {
	  //(closure_op val)-> (machine_op argument sym)
	  current->closure = val;
	  current = arg2;
	  current->type = MACHINE_FLAG;
	  current->flag = ARGUMENT;
	  current->closure = sym;
     }
     return current;
}

closure *clear_list(closure *args)
{
    // This is for the application of atpend arguments
    // which must only be evaluated once.
    if (args->in->type != CONS_PAIR){
	printf("YOU'RE A POOP HEAD: senseless args to clear_list.");
	return nil();
    } else {
	if (!nilp(cheap_cdr(args))) 
	    return cheap_cons(cheap_cons(cleari(), cheap_cons(car(args), nil())), 
			      clear_list(cdr(args)));
	return  cheap_cons(cheap_cons(cleari(), cheap_cons(car(args), nil())), nil());
    }
}

int free_varp(closure *token,
	      closure *accum,
	      frame *current_frame)
{
     if((token->in->type == SYMBOL) &&
	(nilp(assoc(token, accum))))  return 1;
     return 0;
}
	   
closure *find_free_variables(closure *code,
			     frame* current_frame,
			     closure *accum)
{
     if (free_varp(code, accum, current_frame)){
	  //print_closure(code);
	  closure* value = looker_up(code, current_frame);
	  if (!nilp(value)){
	       accum = cheap_acons(code, 
				   cheap_car(value), 
				   accum);
	  }
     } else if (code->in->type == CONS_PAIR && !quotep(code)){
	 accum = find_free_variables(car(code),
				     current_frame, 
				     accum);
	 accum = find_free_variables(cdr(code), 
				     current_frame, 
				     accum);
     }
     return accum;
}

closure *enclose(closure *code, frame *current_frame)
{
//     printf("I'm closing over ");
     //    print_closure(code);
     //printf(":\n");
     closure *ret = copy_closure(code);
     ret->closing = find_free_variables(code, 
					current_frame, 
					code->closing);
     //printf("and the result is\n"); print_closure(ret->closing); printf("\n\n");
     return ret;
};

machine *stack_copy(machine *m){
     machine *ret = new(machine);
     ret->type = MACHINE;
     ret->current_frame = copy_frame(m->current_frame);     
     ret->base_frame = m->base_frame;
     return ret;
}

frame *copy_frame(frame *fram)
{
     // Todo --- this can be made a loop, but not by the compiler.
     if (fram->below == NULL)
	  return fram;
     frame *ret = new_frame(NULL);
     ret->scope = fram->scope;
     ret->rib = fram->rib;
     ret->next = fram->next;
     ret->below = copy_frame(fram->below);
     return ret;
}

closure *fn_lambda_list(closure *fn)
{
     return car(fn); // This may change in the FUTURE! Duhn duhn duuuuuhhhnnnn.
}

operation *fn_instructions(closure *fn)
{
     operation* cl = new(operation);
     cl->type = CLOSURE_OP;
     instruction_list(cdr(fn), cl);
     return cl->next;
}

operation *instruction_list(closure *list, operation *doit)
{
     if(nilp(list)){
	  return NULL;
     } else {
	  operation* cl = new(operation);
	  cl->type = CLOSURE_OP;
 	  cl->closure = car(list);
	  doit->next = cl;
	  cl->next = instruction_list(cdr(list), cl);
	  return cl;
     }
}

frame *new_frame(frame *below)
{
     frame *ret = new(frame);
     ret->type = FRAME;
     ret->below = below;
     ret->scope = nil();
     ret->rib = nil();
     return ret;
}

void print_info(machine *m)
{
     printf("\ninstruction: ");
     if(m->current_frame->next){
     	  print_op(m->current_frame->next);
     }
     if(m->accum){
      	  printf(", accum: ");
     	  print_closure(m->accum);
        }
     print_stack(m->current_frame);
     printf("\n");
     
//printf("\ndon printing\n\n");
 }


int virtual_machine_step(machine *m)
{
     if(DEBUG) print_info(m);
     // If there's no next instruction on this frame, pop it. 
     // If there's no frame below this one, halt (return 1)
     if (m->current_frame->next == NULL){
	  if (m->current_frame->below != NULL){
	       m->current_frame = m->current_frame->below;
	       return 0;
	  } else {
	       return 1; // halt instead of popping the final frame
	  }
     }

     // Pop the current instruction into 'instruction'
     operation *instruction = m->current_frame->next;
     m->current_frame->next = instruction->next;

     // A 'closure-op' is an operation that contains an eight list
     // to be evaluated plainly.
     if (instruction->type == CLOSURE_OP){

	  if (instruction->closure->in->type == BUILTIN){
	       // If it's a builtin function, then we should simply call
	       // the function pointer and let it figure out what to do.
	       builtin fn = instruction->closure->in->builtin_fn;
	       fn(m);

	       
	  } else if (instruction->closure->in->type == CONTINUATION){
	       // A continuation requires looking up the call value
	       // and putting it in accum, before replacing the machine
	       // with the one contained inside the continuation.
	       m->accum = car(looker_up(symbol(string_to_symbol_id(L"a")), 
					m->current_frame));
	       m->base_frame = instruction->closure->in->mach->base_frame;
	       m->current_frame = instruction->closure->in->mach->current_frame;    

	  } else if (instruction->closure->in->type == SYMBOL){
	       // A symbol should be looked up, and the result placed in 
	       // accum. If it is undefined, a signal should be thrown.
	       closure *res =  looker_up(instruction->closure,
					 m->current_frame);
	       if (nilp(res)) {
		    closure *sig = build_signal(cons(string(L"\n\n\nthere's an old man in town\nwho puts his spoon in his mouth\nand he swallows\nbut there's no soup in the bowl\n\n\nerror: I attempted to look up a symbol that was undefined: "), cons(instruction->closure, nil())), m);
		    toss_signal(sig, m);
	       } else {
		    m->accum = car(res);
	       }

	  } else if (!nilp(instruction->closure->closing)){
	       // If the closure has a non-nil closing, then we need to 
	       // add a frame to insert that closing into the scope,
	       // and then try again.

               // new frame, as if a function application
	       m->current_frame = new_frame(m->current_frame);

	       closure* sub = copy_closure(instruction->closure);
	       sub->closing = nil();
	       operation* cl = new(operation);
	       cl->type = CLOSURE_OP;
	       cl->closure = sub;

	       m->current_frame->next = cl;
	       m->current_frame->scope = instruction->closure->closing;

	  } else if (instruction->closure->in->type != CONS_PAIR){
	       // This test is maybe a little dangerous, because it's a
	       // shortcut for characters, numbers, and other 'literals' that
	       // return themselves when evaluated. (3 -> 3)
	       m->accum = instruction->closure;

	  } else if (quotep(instruction->closure)){
	       // If the instruction has been quoted, enclose it.
	       m->accum = enclose(second(instruction->closure), 
				  m->current_frame);
	       // TODO sanity check, is lonely cdr?

	  } else if (car(instruction->closure)->in->symbol_id == CLEAR){
	       // Clear is necessary for atpend. Return
	       // the object that has been tagged to clear.
	       m->accum = car(cdr(instruction->closure));

	  } else if (stringp(instruction->closure)){
	       // This is a bit of a hack, and may change if I have
	       // a better idea. In the meantime, strings are literals.
	       m->accum = instruction->closure;

	  } else {
	       // This is actually the bulk of the work, right here :)
	       // If it's not any of the previous things, then it's
	       // function application. So here we create a different kind
	       // of instruction: the machine flag. Machine flag instructions
	       // are internal notes from the interpreter to itself. In this
	       // case, it's APPLY, which starts the function application 
	       // process once the function istelf has been looked up.

	       operation* fn=new(operation);
	       operation* apply=new(operation);
	       fn->type = CLOSURE_OP;
	       fn->closure = car(instruction->closure);
	       fn->next = apply;
	       apply->type = MACHINE_FLAG;
	       apply->flag = APPLY;
	       apply->closure = cdr(instruction->closure);
	       apply->next = m->current_frame->next;
	       m->current_frame->next = fn;

	  };

     } else if (instruction->type == MACHINE_FLAG){
	  // That's it for ways to interpret simple closures. Now, we
	  // move on to machine flag instructions.

	  if (instruction->flag == APPLY) {
	      // The APPLY step adds a new frame to the stack (unless
	      // there's a tail-call), prepares the 'rib' (which will
	      // become the scope of the new function call), builds the
	      // argument chain (which is a series of instructions that 
	      // evaluate the arguments before actually performing the 
	      // function. 
	      closure *name = cheap_car(assoc(symbol(FUNCTION_NAME), 
					      m->accum->in->info));
	      if (nilp(name)){
		  closure *sig = build_signal(cons(string(L"\n\n\nI am one of the ten thousand things\nbut I cannot forget that I am also an I\nso I hope and act and dream instead\n\n\nI attempted to treat something as a function, but it wasn't:"), cons(m->accum, nil())), m);
		  toss_signal(sig, m);
	      } else {
		  frame* n_frame;
		  if ((m->current_frame->next == NULL) && 
		      (m->current_frame != m->base_frame)){
		      n_frame = m->current_frame;
		  } else {
		      n_frame = new_frame(m->current_frame);
		  }
		  n_frame->rib = m->accum->closing;
		  
		  operation* fn=new(operation);
		  fn->type = MACHINE_FLAG;
		  fn->flag = DO; 
		  fn->closure = m->accum;
		  m->current_frame = n_frame;
		  operation* chain=build_argument_chain(fn_lambda_list(m->accum),
							instruction->closure,
							fn);
		  
		  if(chain == (operation *)-1){
		      closure *sig = build_signal(cons(string(L"\n\n\nClouds drift into my window\nbut they don't drift out again\nsoon the frame will burst\nand the sill will fall\n\n\nerror: too many arguments were given to this function:"), cons(name, nil())), m);
		      toss_signal(sig, m);
		      return 0;
		  } 
		  if (fn->next != NULL){
		      n_frame->next = fn->next;
		      chain->next = fn;
		      fn->next = NULL;
		  } else {
		      n_frame->next = fn;
		  }
	      }

	  } else if (instruction->flag == ARGUMENT){
	       // This one pushes the argument onto the rib, once it's been
	       // evaluated.

	       if (e_argp(instruction->closure)){
		    // If it's an elipsis-arguemnt, then
		    // keep tacking things on to the last
		    // arg.
		    // TODO: This is slow, and might be better done backwards
		    // with an extra 'reverse' at the end.
		    closure *last = cdr(car(m->current_frame->rib));
		    last->in->cons->car = append(car(last),
					     cons(m->accum, nil()));
			 
	       } else {
		    // If it's a normal arg, just do this.
		    m->current_frame->rib = cheap_acons(instruction->closure,
							m->accum,
							m->current_frame->rib);
	       }

	  } else if (instruction->flag == CONTINUE_APPLY){
	       // CONTINUE_APPLY reloads an argument chain when the machine
	       // has had to pause and evaluate some part of it (for asterices)
	       operation* fn=new(operation);
	       fn->type = CLOSURE_OP;
	       operation* boo = build_argument_chain(
		    car(instruction->closure),
		    append(m->accum, cdr(instruction->closure)),
		    fn);
	       boo->next = m->current_frame->next;
	       m->current_frame->next = fn->next;

	  } else if (instruction->flag == ATPEND_APPLY){
	       // Here, we want to evaluate the argument, splice it in,
	       // but NOT evaluate it a second time. So we clear_list it,
	       // unlike above.

	       closure *args = clear_list(m->accum);
	       operation* fn=new(operation);
	       fn->type = CLOSURE_OP;
	       operation* boo = 
		   build_argument_chain(
		       car(instruction->closure),
		       cheap_append(args, cdr(instruction->closure)),
		       fn);
	       boo->next = m->current_frame->next;
	       m->current_frame->next = fn->next;

	  } else if (instruction->flag == DO){

	       operation* ins = fn_instructions(instruction->closure);
	       m->current_frame->next = ins;
	       m->current_frame->scope = m->current_frame->rib;
	  }
     }
     return 0;
}
  


machine* eval(closure *form, machine *m){
     operation* newop = new(operation);
     newop->type = CLOSURE_OP;
     newop->closure = form;
     newop->next = m->current_frame->next;
     m->current_frame->next = newop;
     int i = 0; 
     while (i == 0){
 	  i = virtual_machine_step(m);
	  m = collect();
     }

     return m;
}



#endif

