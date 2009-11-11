#ifndef EIGHT_
#define EIGHT_

#include "stdio.h"
#include "gc.h"

// Machine flags, which trigger the 8 VM to change state.
typedef enum {
  DO, APPLY, CONTINUE_APPLY, ATPEND_APPLY, ARGUMENT, E_ARGUMENT, SIGNAL,
} machine_flag;

// These are the built-in functions which have special meaning.
typedef enum {
     ELIPSIS, ASTERIX, COMMA, QUOTE, ATPEND, CLEAR, //not-as-special
     T,//totally unspecial
     MAX_SPECIAL
} special; 
// eventually, maths will fall in here too.
// will this conflict with namespacing?
// need the typedef?


typedef int symbol_id;

typedef enum  {
  CLOSURE, SYMBOL, CONS_PAIR, NIL, FIXNUM, CHARACTER, INTERNAL, CONTINUATION 
} closure_type;

typedef enum {
     MACHINE_FLAG, CLOSURE_OP
} operation_type;

typedef struct closing_struct closing;
typedef struct closure_struct closure;
typedef struct operation_struct operation;
typedef struct cons_pair_struct cons_pair;
typedef struct frame_struct frame;
typedef struct machine_struct machine;
typedef void (* builtin) (machine*);

/* A closing is a linked list of bindings, symbol to closure. */
struct closing_struct {
     symbol_id  sym;
     closure   *val;
     closing   *next;
};

struct cons_pair_struct {
     closure *car;
     closure *cdr;
};

/* A closure is a data container; it can be atomic or a cons pair.
   The closure also contains binding information for any free variables
   found inside, [and annotation for error handling, documentation, etc?] */

struct closure_struct  {         
     closure_type      type;
     union {
          void        *value; // generic
	  int          fixvalue;
	  char         charvalue;// should probably change for unicode
	  symbol_id    symbol_id;
	  cons_pair   *cons;
          closure     *closee;
          machine     *mach;
     };
     union {
	  void           *annotation;
	  builtin         builtin_fn;
     }; 
     closing        *closed;
};  // if annotation is desired, it could be added here.

/* An operation is a representation of a partial continuation of the 8VM.*/
struct operation_struct {
     operation_type type;
     machine_flag flag;
     closure        *closure;
     operation  *next;
};

/* A frame is one element in the 8VM stack, and keeps track of
   the next operation, the current scope, the 'rib' of values currently
   under construction for the next function application, and the frame
   which is in waiting, cryogenically frozen just below. */
struct frame_struct {
     operation   *next; 
     closing     *scope;
     closing     *rib;
     closure     *signal_handler; 
     frame       *below;
};

struct machine_struct {
     frame *current_frame;
     closure *accum;
     frame *base_frame;
};

//------------------------------------------protos---------------------------//
closure *nil();
closure *quote();
machine *init_8VM();
closure *symbol(symbol_id id);
closure *cons(closure *car, closure *cdr);
closure *car(closure *cons);
closure *cdr(closure *cons);
closure *append(closure *a, closure *b);
closing *closing_append(closing *a, closing *b);
void aappend(closure *val, closing *aclosing);
closure *second(closure* list);
closing *assoc(symbol_id sym, closing* closing);
closing *acons(symbol_id sym, closure *val, closing *aclosing);
closure *looker_up(closure *symbol, frame *aframe, frame *base_frame);
closure *looker_up_internal(symbol_id symbol, frame *aframe, frame *base_frame);
void internal_set(closure *symbol, 
		  closure *value, 
		  frame   *aframe, 
		  frame   *base_frame);
int commap(closure *arg);
int asterixp(closure *arg);
int quotep(closure *sym);
int elipsisp(closure *sym);
int optional_argp(closure *sym);
int e_argp(closure *sym);
operation *build_argument_chain(closure *lambda_list,				    
				closure *arg_list,
				operation* current);
operation *make_arg(closure *sym, closure *val, operation *current);
closure *clear_list(closure *args);
int free_varp(closure *token, 
	      closing *accum, 
	      frame *current_frame);

closing *find_free_variables(closure *code, 
			     frame* current_frame,
			     frame *base_frame,
			     closing *accum);

closure *bclose(closure *code, frame *current_frame, frame *base_frame);
machine *stack_copy(machine *m);
frame *copy_frame(frame *fram);
frame *bottom_frame(frame *fram);
closure *fn_lambda_list(closure *fn);
operation *fn_instructions(closure *fn);
operation *instruction_list(closure *list, operation *doit);
void do_internal(closure *ins, machine *m);
int virtual_machine_step(machine *m);
int eval(closure *form, machine *m);
closing *remove_sym(symbol_id sym, closing *clos);
void combine(closing *a, closing *b, closing **newa, closing **newb, closing **ret);

//print.c
void print_closure(closure *a);
void print_cons(closure *cons);
void print_op(operation* ins);
void print_cont(operation *ins);
void print_assoc(closing *cl);
void print_frame(frame *fm);
void print_stack(frame *fm);
void print_string(closure *a);
void print_string_internal(closure *a);

//basic_commands
closing *basic_commands(closing *start);
void new_basic_commands(machine *m);
closure * build_signal(closure *a, machine *m);
void toss_signal(closure* sig, machine* m);
frame *find_signal_handler(frame *f);

// symbols:
void initialize_symbol_table();
void insert_symbol(char *name, int val);
symbol_id string_to_symbol_id( char *name );
char* symbol_id_to_string(symbol_id sym);


//strings
closure* character(char a);
closure *string(char * str);
int stringp(closure *a);
#endif
