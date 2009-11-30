/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley (diiq, stm31415@gmail.com)

Guarantees that must be upheld:
No operation ever alters it's next, once assigned

TODO 
 sanity checking
 readline
 force --- see if you need it/can write it with leak.

***************************************************************************/

#include "eight.h"
#include "symbols.c"
#include "gc.h"

closure *e_argument;
closure *cleari;

machine *init_8VM()
{
     e_argument = symbol(E_ARGUMENT); 
     e_argument->type = INTERNAL;

     cleari = symbol(CLEAR); 
     cleari->type = INTERNAL;

     insert_symbol("quote", QUOTE);
     insert_symbol("asterix", ASTERIX);
     insert_symbol("atpend", ATPEND);
     insert_symbol("comma", COMMA);
     insert_symbol("...", ELIPSIS);
     insert_symbol("t", T);
     machine *m = (machine *)GC_MALLOC(sizeof(machine));
     frame *cframe = (frame *)GC_MALLOC(sizeof(frame));
     frame *bframe = (frame *)GC_MALLOC(sizeof(frame));
     m->current_frame = bframe;
     m->base_frame = bframe;
     internal_set(symbol(T), symbol(T), bframe, bframe);
     new_basic_commands(m);
     return m;
}	

closure *nil()
{
     closure *nil = (closure*)GC_MALLOC(sizeof(closure)); 
     cons_pair* nil_pair = (cons_pair*)GC_MALLOC(sizeof(cons_pair)); 
     nil->type = NIL;
     nil->cons = nil_pair;
     nil_pair->car = nil;
     nil_pair->cdr = nil;
     return nil;
}

closure *quote()
{
     return symbol(QUOTE);
};


closure *symbol(symbol_id id)
{
     // builds a symbol-closure from an id.
     closure *sym = (closure*) GC_MALLOC(sizeof(closure));
     sym->type = SYMBOL;
     sym->symbol_id = id;
     return sym; 
};

closure *fixnum(int num)
{
     // builds a fixnum-closure from an integer
     closure *sym = (closure*) GC_MALLOC(sizeof(closure));
     sym->type = FIXNUM;
     sym->fixvalue = num;
     return sym; 
};

closure *cons(closure *car, closure *cdr)
{
     // proper behaviour here is to find the common closees, and join them?
 
     closure *pair = (closure*) GC_MALLOC(sizeof(closure));
     pair->type = CONS_PAIR;

     cons_pair *cpair = (cons_pair*) GC_MALLOC(sizeof(cons_pair));

     if (cdr->type == NIL){
	  pair->closed = car->closed;
	  car->closed = NULL;
	  cpair->car = car;
	  cpair->cdr = cdr;
	  pair->cons = cpair;	  
     } else if (car->type == NIL){
	  pair->closed = cdr->closed;
	  cdr->closed = NULL;
	  cpair->car = car;
	  cpair->cdr = cdr;
	  pair->cons = cpair;
     } else {
	  closing *newcar_close = closing_append(car->closed, NULL);
	  closing *newcdr_close = closing_append(cdr->closed, NULL);
	  closing *consclose = NULL;
	  combine(car->closed, cdr->closed, &newcar_close, &newcdr_close, &consclose);
	  car->closed = newcar_close;
	  cdr->closed = newcdr_close;
	  pair->closed = consclose;
	  
	  cpair->car = car;
	  cpair->cdr = cdr;
	  pair->cons = cpair;
     }
     return pair;
};

void combine(closing *a, 
	     closing *b, 
	     closing **newa, 
	     closing **newb,
	     closing **ret)
{
     // TODO WARNING BUG!!! If a contains a concealed closure (q->5
     // ... q->25), and b contains a match to the concealed value (a
     // not unimaginable occurance), then combine will dissapear the
     // q->5, *unconcealed* binding.

     if (a == NULL) return;
     closing *match = assoc(a->sym, *ret);
     if (match!=NULL){
	  // if sym's already in the new closing, ignore it.
	  combine(a->next, b, newa, newb, ret);
     } else {
	  match = assoc(a->sym, b);
	  if (match == NULL){
	       // if it isn't in b, do nothing.
	       combine(a->next, b, newa, newb, ret);
	  } else {
	       if (match->val == a->val){
		    // if it is in b, and they match, add to new and remove from a and b
		    closing * out = (closing *)GC_MALLOC(sizeof(closing));
		    out->next = *ret;
		    out->sym = a->sym;
		    out->val = a->val;
		    *ret = out;
		    *newa = remove_sym(a->sym, *newa);
		    *newb = remove_sym(a->sym, *newb);
		    combine(a->next, b, newa, newb, ret);
	       } else {
		    // if conflicting definitions, ignore.
		    combine(a->next, b, newa, newb, ret);
	       }
	  }
     }
}


// TODO typecheck these.
closure *car(closure *cons)
{
     if (cons->type == NIL) return cons;
     if (cons->closed != NULL){
     	  closure *clo = (closure*) GC_MALLOC(sizeof(closure));
     	  clo->type = cons->cons->car->type;
     	  clo->value = cons->cons->car->value;
	  clo->closed = closing_append(cons->cons->car->closed, 
	  			       cons->closed);
     	  return clo;
     } else {
	  return cons->cons->car;
     }
}

closure *cdr(closure *cons)
{
     if (cons->type == NIL) return cons;
     if (cons->closed != NULL){
     	  closure *clo = (closure*) GC_MALLOC(sizeof(closure));
     	  clo->type = cons->cons->cdr->type;
     	  clo->value = cons->cons->cdr->value;
	  clo->closed = closing_append(cons->cons->cdr->closed, 
				       cons->closed);
     	  return clo;
     } else {
	  return cons->cons->cdr;
     }
}

closure *append(closure *a, closure *b)
{
     // TODO sanity test a & b for listness.
     if (a->type == NIL){
	  return b;
     } else {
	  return cons(car(a), append(cdr(a), b));
     }
};

closing *closing_append(closing *a, closing *b)
{
     // TODO sanity test a & b for listness.
     if (a == NULL){
	  return b;
     } else if (b == NULL){
	  return a;
     } else {
	  closing *clo = (closing*) GC_MALLOC(sizeof(closing));
	  clo->sym = a->sym;
	  clo->val = a->val;
	  clo->next = closing_append(a->next, b);
	  return clo;
     }
}

closing *remove_sym(symbol_id sym, closing *clos)
{
     if (clos == NULL){
	  return NULL;
     } else if (clos->sym == sym) {
	  return remove_sym(sym, clos->next);
     } else {
	  closing *clo = (closing*) GC_MALLOC(sizeof(closing));
	  clo->sym = clos->sym;
	  clo->val = clos->val;
	  clo->next = remove_sym(sym, clos->next);
	  return clo;
     }
}

closure *second(closure* list)
{
     return car(cdr(list));
}

closing *assoc(symbol_id sym, closing* closing)
{
     if (closing == NULL)
	  return NULL;
     if (sym == closing->sym)
	  return closing;
     return assoc(sym, closing->next);
};

closing *acons(symbol_id sym, closure *val, closing *aclosing)
{
     closing *output = (closing *) GC_MALLOC(sizeof(closing));
     closure **reval = (closure**) GC_MALLOC(sizeof(closure *));
     *reval = val;
     output->next = aclosing;
     output->sym = sym;
     output->val = reval;
     return output;
};

void aappend(closure *val, closing *aclosing)
{
  closure *b= append(*(aclosing->val), cons(val, nil()));
  *(aclosing->val) = b;
};

int length(closure *xs)
{
  int i = 0;
  while(xs->type == CONS_PAIR){
    i++;
    xs = cdr(xs);
  }
  return i;
}

 

closure *looker_up(closure *symbol, frame *aframe, frame *base_frame)
{
  closure **ret = s_looker_up(symbol, aframe, base_frame);
  if (ret)
    return *ret;
  return NULL;
};

closure **s_looker_up(closure *symbol, frame *aframe, frame *base_frame)
{
     // I could type-check for symbol-ness, here.
     // But I don't think it'll be an issue.
     closing *local = assoc(symbol->symbol_id, symbol->closed);
     if (local == NULL) 
       return looker_up_internal(symbol->symbol_id, aframe, base_frame);
     return local->val;
};

closure **looker_up_internal(symbol_id symbol, frame *aframe, frame *base_frame)
{
     closing* local =  assoc(symbol, aframe->scope);
     if (local == NULL) {
       if (aframe->below == NULL)
	 return NULL;
       return looker_up_internal(symbol, aframe->below, base_frame);
     }
     return local->val;
};

void internal_set(closure *symbol,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame)
{
     closure **old = looker_up_internal(symbol->symbol_id, aframe, base_frame);
     if (old == NULL){
	  base_frame->scope = acons(symbol->symbol_id,
				    value,
				    base_frame->scope);
     } else {
         *old = value;
     };
}

int commap(closure *arg)
{
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == COMMA)) return 1;
     return 0;
};

int asterixp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == ASTERIX)) return 1;
     return 0;
};
int atpendp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == ATPEND)) return 1;
     return 0;
};
int quotep(closure *sym)
{
     // returns true if argument == *x
     if ((sym->type == CONS_PAIR) &&
	 (car(sym)->type == SYMBOL) &&
	 (car(sym)->symbol_id == QUOTE)) return 1;
     return 0;
};
int elipsisp(closure *sym)
{
     //returns true if sym == ...
     if ((sym->type == SYMBOL) &&
	 (sym->symbol_id == ELIPSIS)) return 1;
     return 0;
}

int optional_argp(closure *sym)
{
     if ((sym->type == CONS_PAIR) &&
	 (car(sym)->type == SYMBOL) &&
	 (car(sym)->symbol_id != QUOTE))  return 1;
     return 0;
};

int e_argp(closure *sym)
{
     if (((sym->type == INTERNAL) && 
	  (sym->symbol_id == E_ARGUMENT)) ||
	 ((sym->type == CONS_PAIR) &&
	  ((second(sym)->type == INTERNAL) && 
	   (second(sym)->symbol_id == E_ARGUMENT)))) return 1;
     return 0;
}


operation *build_argument_chain(closure *lambda_list,
				closure *arg_list,
				operation *current)
{
     // builds the operations necessary to evaluate the arguments of a fn.
     if (lambda_list->type == NIL) {
	  if (arg_list->type != NIL) {
	       printf("Too many args!"); 
	  }
	  // If we're out of lambda list, we're done!
	  return current;
     } else if (asterixp(car(arg_list))){
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))
	  operation *tnext = (operation*)GC_MALLOC(sizeof(operation));
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = (operation*)GC_MALLOC(sizeof(operation));
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = CONTINUE_APPLY;
	  bnext->closure = cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (atpendp(car(arg_list))){
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))
	  operation *tnext = (operation*)GC_MALLOC(sizeof(operation));
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = (operation*)GC_MALLOC(sizeof(operation));
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = ATPEND_APPLY;
	  bnext->closure = cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (elipsisp(car(lambda_list))){
	  // the trick here is that you have to signal yourself
	  // that you're elipsising, even THROUGH a continue_apply
	  closure *argm;
	  if (quotep(second(lambda_list))){
	       // if the next arg is quoted, keep the quote.
	       argm = cons(cons(quote(), cons(e_argument, nil())), nil());
	  } else {
	       argm = cons(e_argument, nil());
	  }
	  if (arg_list->type == NIL){
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
	  if (arg_list->type != NIL) {
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
	  if (e_argp(car(lambda_list)) && (cdr(arg_list)->type != NIL)){
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
     operation* arg  = (operation *)GC_MALLOC(sizeof(operation));
     operation* arg2 = (operation *)GC_MALLOC(sizeof(operation));
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
	       current->closure = cons(quote(),cons(val, nil()));
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
     if (args->type != CONS_PAIR){
	  printf("YOU'RE A POOP_HEAD");
	  return nil();
     } else {
	  if (cdr(args)->type != NIL) 
	       return cons(cons(cleari, cons(car(args), nil())), 
			   clear_list(cdr(args)));
	  return  cons(cons(cleari, cons(car(args), nil())), nil());
     }
}

int free_varp(closure *token,
	      closing *accum,
	      frame *current_frame){
     if((token->type == SYMBOL) &&
	(assoc(token->symbol_id, accum) == NULL))  return 1;
     return 0;
}
	   
closing *find_free_variables(closure *code,
			     frame* current_frame,
			     frame* base_frame,
			     closing *accum)
{
     if (free_varp(code, accum, current_frame)){
	  closure* value = looker_up(code, current_frame, base_frame);
	  if (value != NULL){
	       accum = acons(code->symbol_id, value, accum);
	  }
     } else if ((code->type == CONS_PAIR) &&
		(car(code)->symbol_id != QUOTE)){
	  accum = find_free_variables(car(code), current_frame, base_frame, accum);
	  accum = find_free_variables(cdr(code), current_frame, base_frame, accum);
     }
     return accum;
}

closure *bclose(closure *code, frame *current_frame, frame *base_frame)
{
     closure *ret = (closure *)GC_MALLOC(sizeof(closure));
     ret->type = code->type;
     ret->value = code->value;
     ret->closed = find_free_variables(code, current_frame, base_frame, code->closed);
     //     code->closed = find_free_variables(code, current_frame, base_frame, code->closed);
     // OK, I am not at all sure about this choice; should ',x release all
     // previous closure, and reclose, or should it only close over what
     // remains free? For the moment, the latter.
     // Also, should this produce a COPY of the closure, or destructively
     // alter? It must destructively alter, or the closure algebra falls to bits.
     return ret;
};

machine *stack_copy(machine *m){
     machine *ret = (machine *)GC_MALLOC(sizeof(machine));
     ret->current_frame = copy_frame(m->current_frame);     
     ret->base_frame = bottom_frame(ret->current_frame);
     return ret;
}

frame *copy_frame(frame *fram)
{
     if (fram->below == NULL)
	  return fram;
     frame *ret = (frame *)GC_MALLOC(sizeof(frame));
     ret->scope = fram->scope;
     ret->rib = fram->rib;
     ret->next = fram->next;
     ret->below = copy_frame(fram->below);
     return ret;
}

frame *bottom_frame(frame *fram)
{
     if (fram->below == NULL)
	  return fram;
     return bottom_frame(fram->below);
}

closure *fn_lambda_list(closure *fn)
{
     return car(fn);
}
operation *fn_instructions(closure *fn)
{
     operation* cl = (operation *)GC_MALLOC(sizeof(operation));
     instruction_list(cdr(fn), cl);
     return cl->next;
}

operation *instruction_list(closure *list, operation *doit)
{
     if(list->type == NIL){
	  return NULL;
     } else {
	  operation* cl = (operation *)GC_MALLOC(sizeof(operation));
	  cl->type = CLOSURE_OP;
 	  cl->closure = car(list);
	  doit->next = cl;
	  cl->next = instruction_list(cdr(list), cl);
	  return cl;
     }
}


int virtual_machine_step(machine *m)
{
       // pop/halt condition
     if (m->current_frame->next == NULL){
	  if (m->current_frame->below != NULL){
	       m->current_frame = m->current_frame->below;
	       return 0;
	  } else {
	       return 1; // halt instead of popping the final frame
	  }
     }
     operation *instruction = m->current_frame->next;
     m->current_frame->next = instruction->next;
     if (instruction->type == CLOSURE_OP){
	  if (instruction->closure->type == INTERNAL){
	       builtin fn = instruction->closure->builtin_fn;
	       fn(m);
	  } else if (instruction->closure->type == CONTINUATION){
	    m->accum = looker_up(symbol(string_to_symbol_id("a")), 
					     m->current_frame,
					     m->base_frame);
	       m->base_frame = instruction->closure->mach->base_frame;
	       m->current_frame = instruction->closure->mach->current_frame;		     
	  } else if (instruction->closure->type == SYMBOL){
	       m->accum = looker_up(instruction->closure,
				    m->current_frame,
				    m->base_frame);
	       if (m->accum == NULL) {
		    closure *sig = build_signal(cons(string("\n\n\nthere's an old man in town\nwho puts his spoon in his mouth\nand swallows\nbut there's nothing there\n\n\nerror: I attempted to look up a symbol that was undefined: "), instruction->closure), m);
		    toss_signal(sig, m);
	       }
	  } else if (instruction->closure->closed != NULL){
               // new frame, as if a function application
	       frame* new_frame = (frame *)GC_MALLOC(sizeof(frame));
	       closure* sub = (closure *)GC_MALLOC(sizeof(closure));
	       operation* cl = (operation *)GC_MALLOC(sizeof(operation));
	       sub->value = instruction->closure->value;
	       sub->type = instruction->closure->type;
	       cl->type = CLOSURE_OP;
	       cl->closure = sub;
	       new_frame->next = cl;
	       new_frame->scope = instruction->closure->closed;
	       new_frame->below = m->current_frame;
	       m->current_frame = new_frame;
	  } else if (instruction->closure->type != CONS_PAIR){
	       m->accum = instruction->closure;
	  } else if (car(instruction->closure)->symbol_id == QUOTE){
	       m->accum = bclose(second(instruction->closure), m->current_frame,
		    m->base_frame);
	       // TODO sanity check, is lonely cdr?
	  } else if (car(instruction->closure)->symbol_id == CLEAR){
	       m->accum = car(cdr(instruction->closure));
	  } else if (stringp(instruction->closure)){
	       m->accum = instruction->closure;
	  } else {
	       // fn application
	       operation* fn=(operation*)GC_MALLOC(sizeof(operation));
	       operation* apply=(operation*)GC_MALLOC(sizeof(operation));
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
	  if (instruction->flag == APPLY) {
	       frame* new_frame;
	       if ((m->current_frame->next == NULL) && 
		   (m->current_frame != m->base_frame)){
		    new_frame = m->current_frame;
	       } else {
		    new_frame = (frame*)GC_MALLOC(sizeof(frame));
		    new_frame->scope = NULL;
		    new_frame->below = m->current_frame;
	       }
	       new_frame->rib = m->accum->closed;

	       operation* fn=(operation*)GC_MALLOC(sizeof(operation));
	       operation* chain=build_argument_chain(fn_lambda_list(m->accum),
						     instruction->closure,
						     fn);
	       fn->type = MACHINE_FLAG;
	       fn->flag = DO;
	       fn->closure = m->accum;
	       if (fn->next != NULL){
		    new_frame->next = fn->next;
		    chain->next = fn;
		    fn->next = NULL;
	       } else {
		    new_frame->next = fn;
	       }

	       m->current_frame = new_frame;
	  } else if (instruction->flag == ARGUMENT){
	       if ((instruction->closure->type == INTERNAL) && 
		   (instruction->closure->symbol_id == E_ARGUMENT)){
		    aappend(m->accum, m->current_frame->rib);
	       } else {
		    symbol_id a = instruction->closure->symbol_id;
		    m->current_frame->rib = acons(a,
						  m->accum,
						  m->current_frame->rib);
	       }
	  } else if (instruction->flag == CONTINUE_APPLY){
	       operation* fn=(operation*)GC_MALLOC(sizeof(operation));
	       operation* boo = build_argument_chain(car(instruction->closure),
						     append(m->accum, cdr(instruction->closure)),
						     fn);
	       boo->next = m->current_frame->next;
	       m->current_frame->next = fn->next;
	  } else if (instruction->flag == ATPEND_APPLY){
	       closure *args = clear_list(m->accum);
	       operation* fn=(operation*)GC_MALLOC(sizeof(operation));
	       operation* boo = build_argument_chain(car(instruction->closure),
						     append(args, cdr(instruction->closure)),
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
  



int eval(closure *form, machine *m){
     operation* newop = (operation *)GC_MALLOC(sizeof(operation));
     newop->type = CLOSURE_OP;
     newop->closure = form;
     newop->next = m->current_frame->next;
     m->current_frame->next = newop;
     int i = 0; 
     while (i == 0){
	  i = virtual_machine_step(m);
     }
     return 0;
}


