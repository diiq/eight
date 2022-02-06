/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley

***************************************************************************/
#ifndef EIGHT_MAIN
#define EIGHT_MAIN


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "eight.h"


machine *init_8VM()
{
    init_memory(); 
    machine *m = new_machine_space();

    // The base frame is the only frame without a ->below
    frame *bframe = new_frame(NULL); 
    m->current_frame = bframe;
    m->base_frame = bframe;

    insert_symbol(L"'", QUOTE);
    insert_symbol(L"*", ASTERIX);
    insert_symbol(L"@", ATPEND);
    insert_symbol(L",", COMMA);
    insert_symbol(L"...", ELIPSIS);
    insert_symbol(L"t", T);
    insert_symbol(L"LEAKED", LEAKED);
    insert_symbol(L"help", HELP);
    insert_symbol(L"function-name", FUNCTION_NAME);

    // t represents true and  evaluates to itself.
    internal_set(symbol(T), symbol(T), bframe, bframe);

    intern_builtin_functions(m); // see basic_commands.c

    return m;
}	


closure *cleari()
{
    // clear is used when arguments are atpended; they have to pass
    // through an evaulation step unaltered --- it's like quote used to
    // be.
    closure *cleari = symbol(CLEAR); 
    cleari->in->type = INTERNAL;
    return cleari;
}

closure *e_argument() 
{
    // e_argument is used when ... is encounted in a lambda list. It
    // signals to the virtual machine that the current argument ought
    // to be appended to the previous one.
    closure *e_argument = symbol(E_ARGUMENT); 
    e_argument->in->type = INTERNAL;
    return e_argument;
}


int nilp(closure *x)
{
    // Returns true is arg is nil.
    if(x->in->type == NIL) return 1;
    return 0;
}

closure *symbol(symbol_id id)
{
    // Builds a symbol from an id.
    closure *sym = new_closure_space();
    sym->in = new_object_space();
    sym->in->type = SYMBOL;
    sym->type = OBJ;
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
    closure *ret = new_closure_space();
    ret->in = new_object_space();
    ret->in->type = NUMBER;
    ret->type = OBJ;
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


int equal(closure *a, closure *b)
{
    if ((a==b) ||
	(nilp(a) && nilp(b)) ||
	((a->in->type == b->in->type) && (a->in->obj == b->in->obj)))
	return 1;
    return 0;
}


int length(closure *xs)
{
    int i = 0;
    while(!nilp(xs)){
	i++;
	xs = cdr(xs);
    }
    return i;
}

closure *looker_up(closure *sym, frame *current_frame, frame *base_frame)
{
    // note that the value itself is the car of
    // what is returned; by setting the car of
    // what is returned, you can alter the value
    // destructively.
    //closure *local = nil();//lassoc(sym, sym->closing);
    //if (nilp(local))
    //if (nilp(local) || leakedp(local))
    closure *local = table_lookup(sym, current_frame->scope);
    if (leakedp(local))
    	local = looker_up(sym, current_frame->below, base_frame);
    else if (nilp(local))
	local = table_lookup(sym, base_frame->scope);
    //print_closure(local);printf("\n");
    return local;
};
 

void internal_set(closure *sym,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame)
{
    closure *old = looker_up(sym, aframe, base_frame);
    if (nilp(old)){
	table_insert(sym, value, base_frame->scope);
    } else {
	memcpy(old->in->cons->car, value, sizeof(closure));
    }
}

int commap(closure *arg)
{
    // returns true if argument == ,x
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
    // returns true if argument == @x
    if ((arg->in->type == CONS_PAIR) &&
	(cheap_car(arg)->in->type == SYMBOL) &&
	(cheap_car(arg)->in->symbol_id == ATPEND)) return 1;
    return 0;
};

int quotep(closure *sym)
{
    // returns true if argument == 'x
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
    // returns true if argument == (x a) or ('x a)
    if ((sym->in->type == CONS_PAIR) &&
	(cheap_car(sym)->in->symbol_id != QUOTE &&
	 ((cheap_car(sym)->in->type == SYMBOL) || 
	  quotep(cheap_car(sym))))) return 1;
    return 0;
};

int e_argp(closure *sym)
{
    if(sym->in->type == CONS_PAIR){
	closure *asym = cheap_car(sym);
	if (((asym->in->type == INTERNAL) && 
	     (asym->in->symbol_id == E_ARGUMENT)))
	    return 1;
	
	asym = cheap_cdr(sym);
	if (asym->in->type == CONS_PAIR) {
	    asym = cheap_car(asym);
	    if (asym->in->type == CONS_PAIR) {
		asym = cheap_car(asym);
		if (((asym->in->type == INTERNAL) && 
		     (asym->in->symbol_id == E_ARGUMENT)))
		    return 1;
	    }
	}
    }
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
    } else if (asterixp(car(arg_list))){
	// If the first argument is a *arg, then we need to grab
	// the actual argument, which is the ((asterix HERE) ...)
	// then we build a continue-apply structure so that we can 
	// pause and evaluate the argument.
	// accum->(closure_op, x)->(continue_apply,
	//                          (lambda_list . arg_list->cdr))

	operation *tnext = new_operation_space();
	tnext->type = CLOSURE_OP;
	tnext->closure = second(car(arg_list));
	current->next = tnext;
	current = tnext;

	operation *bnext = new_operation_space();
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
	operation *tnext = new_operation_space();
	tnext->type = CLOSURE_OP;
	tnext->closure = second(car(arg_list));
	current->next = tnext;
	current = tnext;

	operation *bnext = new_operation_space();
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
	    argm = cheap_list(1, quote(cheap_list(2, e_argument(), 
				      second(second(lambda_list)))));
	} else {
	    argm = cheap_list(1, list(2, e_argument(), second(lambda_list)));
	}
	if(DEBUG) {
	    printf("beginning the building e arg.");
	    print_closure(argm);
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
	    if(DEBUG) printf("building e arg.");
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
    operation* arg  = new_operation_space();
    operation* arg2 = new_operation_space();
    current->next = arg;
    current = arg;
    current->type = CLOSURE_OP;
    current->next = arg2;
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
			     frame* base_frame, 
			     closure *accum)
{
    if (free_varp(code, accum, current_frame)){
	//print_closure(code);
	closure* value = looker_up(code, current_frame, base_frame);
	if (!nilp(value)){
	    accum = cheap_acons(code, 
				cheap_car(value), 
				accum);
	}
    } else if (code->in->type == CONS_PAIR){
	accum = find_free_variables(car(code),
				    current_frame,
				    base_frame,
				    accum);
	accum = find_free_variables(cdr(code), 
				    current_frame,
				    base_frame,
				    accum);
    }
    return accum;
}

closure *enclose(closure *code, frame *current_frame, frame* base_frame)
{
    closure *ret = copy_closure(code);
    ret->closing = find_free_variables(code, 
				       current_frame,
				       base_frame, 
				       nil());
    /* if(!nilp(ret->closing)){ */
    /* 	printf("I'm closing over "); */
    /* 	print_closure(code); */
    /* 	printf(":\n"); */
    /* 	printf("and the result is\n"); 
	print_closure(ret->closing); 
	printf("\n\n"); */
    /* } */
    return ret;
};

machine *stack_copy(machine *m){
    machine *ret = new_machine_space();
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
    ret->function = fram->function;
    ret->below = copy_frame(fram->below);
    return ret;
}

closure *fn_lambda_list(closure *fn)
{
    return car(fn); // This may change in the FUTURE! Duhn duhn duuuuuhhhnnnn.
}

operation *fn_instructions(closure *fn)
{
    operation* cl = new_operation_space();
    cl->type = CLOSURE_OP;
    instruction_list(cdr(fn), cl);
    return cl->next;
}

operation *instruction_list(closure *list, operation *doit)
{
    if(nilp(list)){
	return NULL;
    } else {
	operation* cl = new_operation_space();
	cl->type = CLOSURE_OP;
	cl->closure = car(list);
	doit->next = cl;
	cl->next = instruction_list(cdr(list), cl);
	return cl;
    }
}

frame *new_frame(frame *below)
{
    frame *ret = new_frame_space();
    ret->type = FRAME;
    ret->below = below;
    ret->scope = new_symbol_table();
    ret->rib = new_symbol_table();
    ret->function = nil();
    return ret;
}

void print_info(machine *m)
{
    printf("\nnext instruction: ");
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


frame *tail_call_optimize(machine *m)
{
    frame* n_frame;

    if (m->current_frame == m->base_frame){
	n_frame = new_frame(m->current_frame);
	m->current_frame = n_frame;
	return n_frame;
    }

    symbol_table *ascope = m->current_frame->scope;
    symbol_table *arib = m->current_frame->rib;
    closure *ahandler = m->current_frame->signal_handler;
    if ((m->current_frame->next == NULL) &&
	(m->current_frame->signal_handler == NULL) &&
    	(m->current_frame != m->base_frame) &&
    	(m->current_frame->below != m->base_frame)){
    	while ((m->current_frame->below->next == NULL) &&
	       (m->current_frame->below->signal_handler == NULL) &&
    	       (m->current_frame->below != m->base_frame)){
    	    m->current_frame = m->current_frame->below;
	}
    	n_frame = m->current_frame;
	n_frame->signal_handler = ahandler;
	n_frame->rib = arib;
	n_frame->scope = ascope;

    } else {
 	n_frame = new_frame(m->current_frame);
    }

    m->current_frame = n_frame;
    return n_frame;
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
				     m->current_frame, m->base_frame));
	    m->base_frame = instruction->closure->in->mach->base_frame;
	    m->current_frame = instruction->closure->in->mach->current_frame;    

	} else if (instruction->closure->in->type == SYMBOL){
	    // A symbol should be looked up, and the result placed in 
	    // accum. If it is undefined, a signal should be thrown.
	    closure *res = lassoc(instruction->closure, 
				  instruction->closure->closing);
	    if (nilp(res) || leakedp(res))
		res = looker_up(instruction->closure,
				m->current_frame, 
				m->base_frame);
	    if (nilp(res)) {
              //print_info(m);
              closure *sig = build_signal(cons(string(L"\n\n\nthere's an old man in town\nwho puts his spoon in his mouth\nand he swallows\nbut there's no soup in the bowl\n\n\nerror: I attempted to look up a symbol that was undefined: "), cons(instruction->closure, nil())), m);
              toss_signal(sig, m);
              
              //error();
	    } else {
		m->accum = car(res);
	    }

	} else if (!nilp(instruction->closure->closing)){
	    // If the closure has a non-nil closing, then we need to 
	    // add a frame to insert that closing into the scope,
	    // and then try again.


	    symbol_table *as;
	    // new frame, as if a function application
	    as = table_union(closing_to_table(instruction->closure->closing),
			     m->current_frame->scope);
	    	    
	    tail_call_optimize(m);
	    
	    closure* sub = copy_closure(instruction->closure);
	    sub->closing = nil();
	    operation* cl = new_operation_space();
	    cl->type = CLOSURE_OP;
	    cl->closure = sub;
	    
	    m->current_frame->next = cl;
	    m->current_frame->scope = as;

	} else if (instruction->closure->in->type != CONS_PAIR){
	    // This test is maybe a little dangerous, because it's a
	    // shortcut for characters, numbers, and other 'literals' that
	    // return themselves when evaluated. (3 -> 3). It can't be
	    // a symbol because we've already checked for that.
	    m->accum = instruction->closure;

	} else if (quotep(instruction->closure)){
	    // If the instruction has been quoted, enclose it.
	    m->accum = enclose(second(instruction->closure), 
			       m->current_frame, m->base_frame);
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
	    // process once the function itself has been looked up.
	    operation* fn=new_operation_space();
	    operation* apply=new_operation_space();
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
	    //print_closure(name);
	    if (nilp(name)){
		closure *sig = build_signal(cons(string(L"\n\n\nI am one of the ten thousand things\nbut I cannot forget that I am also an I\nso I hope and act and dream instead\n\n\nI attempted to treat something as a function, but it wasn't:"), cons(m->accum, nil())), m);
		toss_signal(sig, m);
	    } else {

		symbol_table *as = m->current_frame->scope;
		frame *n_frame = tail_call_optimize(m);
		n_frame->scope = as;

		n_frame->function = m->accum;
		operation* fn=new_operation_space();
		fn->type = MACHINE_FLAG;
		fn->flag = DO; 
		fn->closure = m->accum;
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
		//print_closure(m->accum);printf("\n");
		//print_closure(m->accum->closing);printf("\n");
		closure *last = table_lookup(second(instruction->closure),
					     m->current_frame->rib);
		table_insert(second(instruction->closure),
			     append(car(last), cons(m->accum, nil())),
			     m->current_frame->rib);
		//print_closure(last->in->cons->car);printf("\n");
		//print_closure(last->in->cons->car->closing);printf("\n");
		//printf("\n\n");
	    } else {
		// If it's a normal arg, just do this.
		table_insert(instruction->closure,
			     m->accum,
			     m->current_frame->rib);
	    }

	} else if (instruction->flag == CONTINUE_APPLY){
	    // CONTINUE_APPLY reloads an argument chain when the machine
	    // has had to pause and evaluate some part of it (for asterices)
	    operation* fn=new_operation_space();
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
	    operation* fn=new_operation_space();
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
	    m->current_frame->rib = new_symbol_table();
	}
    }
    return 0;
}
  


machine* eval(closure *form, machine *m){
    operation* newop = new_operation_space();
    newop->type = CLOSURE_OP;
    newop->closure = form;
    newop->next = m->current_frame->next;
    m->current_frame->next = newop;
    int i = 0; 
    while (i == 0){
	i = virtual_machine_step(m);
	//if (GARBAGE_COLLECT) m = collect();
    }

    return m;
}



#endif

