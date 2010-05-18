#include "eight.h"
#include <math.h>

closure* character(wchar_t a)
{
     closure *ret = new(closure);
     ret->type = CHARACTER;
     ret->character = a;
     ret->closing = nil();
     return ret;
}

closure *string(wchar_t * str)
{
     if(*str == 0)
	  return nil();
     return cons(character(*str), string(str+1));
}

int stringp(closure *a)
{
     if ((a->type == CONS_PAIR) && (car(a)->type == CHARACTER))
	  return stringp_internal(a);
     return 0;
}

int stringp_internal(closure *a)
{
     if (!nilp(a))
	  return 1;
     if ((a->type == CONS_PAIR) && (car(a)->type == CHARACTER))
	  return stringp_internal(cdr(a));
     return 0;
}

/* closure *string_to_number(closure *a){ */
/*     if (!stringp(a)) */
/* 	error(121, 121, "Oh man, I tried to convert a number from a non-string."); */
/*     int l = length(a); */
/*     int num = 0; */
/*     int i; */
/*     for(i=l; i>0; i--){ */
/* 	num += (car(a)->num) * pow(10,i); */
/* 	a = cdr(a); */
/*     } */
/*     return number(num); */

/* } */


