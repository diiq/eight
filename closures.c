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
#ifndef CLOSURES
#define CLOSURES

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


closure *copy_closure(closure *x)
{
     closure *newx = new(closure);
     memcpy(newx, x, sizeof(closure));
     return newx;
}


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




closure *cheap_car(closure *x)
{
     if (nilp(x)) return x;
     return x->in->cons->car;
}

closure *car(closure *x)
{
     if (nilp(x)) return x;
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


closure *cheap_cdr(closure *x)
{
     if (nilp(x)) return x;
     return x->in->cons->cdr;
}
	       

closure *cdr(closure *x)
{
     if (nilp(x)) return x;
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


closure *list(int num, ...)
{
    va_list lis;
    closure *rlis[num];
    closure *ret = nil();
    int i = 0;

    va_start ( lis, num );
    for(i=0; i<num; i++){
	rlis[i] = va_arg ( lis, closure* );
    }
   
    for(i=num-1; i>=0; i--){
	ret = cons(rlis[i], ret);
    }
    return ret;
}

closure *cheap_list(int num, ...)
{
    va_list lis;
    closure *rlis[num];
    closure *ret = nil();
    int i = 0;

    va_start ( lis, num );
    for(i=0; i<num; i++){
	rlis[i] = va_arg ( lis, closure* );
    }
   
    for(i=num-1; i>=0; i--){
	ret = cheap_cons(rlis[i], ret);
    }
    return ret;
}

closure *second(closure* list)
{
     return car(cdr(list));
}

closure *cheap_acons(closure *sym, closure *val, closure *closing)
{
     closure *ret =  cheap_cons(
	  cheap_cons(sym, cheap_cons(val, nil())), 
	  closing);
     return ret;
};

closure *lassoc(closure *sym, closure* closing)
{
    while(!nilp(closing)) {
	if ((sym->in->symbol_id == 
	     cheap_car(cheap_car(closing))->in->symbol_id) &&
	    !leakedp(cheap_cdr(cheap_car(closing)))) {
	 return cheap_cdr(cheap_car(closing));
     }
     closing = cheap_cdr(closing);
    }
    return nil();
};

closure *assoc(closure *sym, closure* closing)
{
    while (!nilp(closing)) {
	if (equal(sym, cheap_car(cheap_car(closing)))) {
	    return cheap_cdr(cheap_car(closing));
	}
	closing = cheap_cdr(closing);
    }
    return nil();
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

closure *append(closure *a, closure *b)
{
     // TODO sanity test a & b for listness.
     if (nilp(a)){
	  return b;
     } else {
	  return cons(car(a), append(cdr(a), b));
     }
};

closure *last(closure *x)
{
    while(!nilp(cdr(x))){
	x = cdr(x);
    }
    return car(x);
}

#endif
