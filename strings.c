#include "eight.h"

closure* character(char a)
{
     closure *ret = (closure *)GC_MALLOC(sizeof(closure));
     ret->type = CHARACTER;
     ret->charvalue = a;
     return ret;
}

closure *string(char * str)
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
     if (a->type == NIL)
	  return 1;
     if ((a->type == CONS_PAIR) && (car(a)->type == CHARACTER))
	  return stringp_internal(cdr(a));
     return 0;
}
