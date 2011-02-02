#ifndef EIGHT_HEADER
#define EIGHT_HEADER

#include <stdio.h>
#include <locale.h>
#include <wchar.h> 

typedef int symbol_id;
typedef enum { a, b, c } obj_type;
typedef struct cons_cell_struct cons_cell; 



typedef struct {
    obj_type type;
    int reference_count;
    closure *info; 
    union {
	void      *builtin_fn;
	void      *c_object;
        machine   *continuation;
	symbol_table *table;

	symbol_id  symbol_id;
	wchar_t    character;
	int        num;

	cons_cell *cons;
	void      *obj;
    };
} object;

typedef struct {
    obj_type type;
    int reference_count;
    object *closed;
} closure;

typedef struct {
    obj_type type;
    int reference_count;
    closure *car;
    closure *cdr;
} cons_cell_struct;

#endif
