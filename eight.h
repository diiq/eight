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

#ifndef EIGHT_HEADER
#define EIGHT_HEADER

#include <stdio.h>
#include <locale.h>
#include <wchar.h> 

// trying to move towards good unicode support, but
//I need people who *use* unicode support to give me feedback.


#define new(x) (x *)allocate(sizeof(x))

int DEBUG = 0;
int GARBAGE_COLLECT = 1; 
// Garbage collection will NOT occur when this is 0.

typedef struct closure_struct closure;
typedef int symbol_id;
typedef struct operation_struct operation;
typedef struct frame_struct frame;

typedef struct symbol_table_struct symbol_table;

// Every object that will need to be garbage collected needs a type-flag. 
typedef enum  {
    EMPTY, REFERENCE,   // These two are only used by the garbage collector,
    NIL, CONS_PAIR, NUMBER, INTERNAL,// these are traditional eight objects,
    CHARACTER, SYMBOL, CONTINUATION, 
    BUILTIN, C_OBJECT, TABLE,         // these hold references to c objects.
    SYMBOL_TABLE,
    DREF,                 // All eight objects are wrapped by a DREF struct
    MACHINE_FLAG, CLOSURE_OP,                  // these are operation types
    MACHINE, FRAME, CONS_CELL                            // aaaand the rest.
} obj_type;


// Machine flags are used to tag operations for special treatment by
// *the machine*. No one survives *the machine*. Handling these makes
// up the second half of virtual_machine_step in eight.c. 
typedef enum {
    APPLY, 
    CONTINUE_APPLY,
    ATPEND_APPLY,
    ARGUMENT, 
    E_ARGUMENT,
    DO, 
    SIGNAL, 
} machine_flag;


// These are the core special symbols of eight; each of these has a
// particular meaning hard-coded in to eight.
typedef enum {
    ELIPSIS, 
    ASTERIX, 
    COMMA,
    QUOTE,
    ATPEND,
    LEAKED,
    HELP,
    T,
    CLEAR,
    FUNCTION_NAME
} special; 


// An operation represents the next thing that should happen in a
// given stack frame. It may be be a closure or a machine flag, or
// both. Operations form singly-linked lists.
struct operation_struct{
     obj_type type;
     machine_flag flag;
     closure *closure;
     operation *next;
};


// A frame represents a set of instructions under a common variable
// scope. The frame structure keeps track of a list of operations, a
// scope (which is currently active), a rib (which accumulates
// arguments for the next DO operation, a signal handler when
// appropriate, and the function currently being applied. Frames form
// linked lists.
struct frame_struct {
    obj_type type;
    operation   *next; 
    closure     *rib;
    closure     *scope;
    closure     *signal_handler; 
    closure     *function;
    frame       *below;
};

// A machine contains a list of frames, the base (global) frame, and
// an accumulator. The machine is also the root node of the whole
// memory tree (see memory.c).
typedef struct {
     obj_type type;
     frame *current_frame;
     frame *base_frame;
     closure *accum;
} machine;

typedef void (* builtin) (machine*);


// Cons cells simply pair up two closures. Having them in a separate
// structure, one extra reference away from the dref struct, is important
// to ensuring the closure algebra works properly.
typedef struct {
     obj_type type;
     closure *car;
     closure *cdr;
} cons_cell;

// Here's the fancy thing itself! doubleref holds an info-object and a
// useful thing --- a cons cell, a number, or whatever.
// WARNING! This is a doubleref struct, but its type, as far as the
// garbage collector is concerned is NOT DREF.
typedef struct {
    obj_type type;
    closure *info;
    union {
	void      *builtin_fn;
	void      *c_object;
	machine   *mach;
	symbol_table *table;
	symbol_id  symbol_id;
	wchar_t    character;
	int        num;
	cons_cell *cons;
	void      *obj;
    };
} doubleref;


// closures are simply an extra wrapper around doubleref. Every
// doubleref hides behind a closure object. This separation of closing
// and object is necessary to the closure algebra.
// WARNING! This is a closure struct, but its type, as far as the
// garbage collector is concerned is DREF.
struct closure_struct {
    obj_type   type;
    closure   *closing;
    doubleref *in;
};

struct symbol_table_struct {
    obj_type type;
    closure **array;
    int size;
    int entries;
};

//------------------------ MEMORY.C -----------------------//

machine *collect();
machine * init_memory();
void *allocate(int size);

// ----------------------- SYMBOLS.C ----------------------//

//symbol table
void initialize_symbol_table();
void insert_symbol(wchar_t *name, int val);
symbol_id string_to_symbol_id(wchar_t *name);
wchar_t* symbol_id_to_string(symbol_id sym);
closure* string_to_symbol(closure *a);

//---------------------- ??? (EIGHT.C) --------------------//

closure *nil();
closure *symbol(symbol_id id);
closure *quote(closure *x);
closure *number(int num);


int nilp(closure *x);
int equal(closure *a, closure *b);
int commap(closure *arg);
int asterixp(closure *arg);
int quotep(closure *sym);
int elipsisp(closure *sym);
int optional_argp(closure *sym);
int e_argp(closure *sym);
int leakedp(closure *x);

int length(closure *a);


//--------------------- CLOSURES.C -----------------------//

closure *copy_closure(closure *x);

void combine(closure *a, 
	     closure *b, 
	     closure **newa, 
	     closure **newb,
	     closure **ret);

closure *rectify_closing(closure *closed);
closure *rectify_closing_i(closure *closed, closure *closing, closure *ret);

closure *cheap_cons(closure *car, closure *cdr);
closure *cons(closure *car, closure *cdr);

closure *cheap_car(closure *x);
closure *car(closure *x);

closure *cheap_cdr(closure *x);
closure *cdr(closure *x);

closure *cheap_list(int num, ...);
closure *list(int num, ...);

//closure *cheap_second(closure* list);
closure *second(closure* list);

closure *cheap_acons(closure *sym, closure *val, closure *closing);
closure *assoc(closure *sym, closure *closing);

closure *cheap_append(closure *a, closure *b);
closure *append(closure *a, closure *b);

closure *last(closure *list);

//-------------------------- EIGHT.C ----------------------------//

// closings
int free_varp(closure *token,
	      closure *accum,
	      frame *current_frame);
closure *find_free_variables(closure *code,
			     frame* current_frame,
			     frame* base_frame,
			     closure *accum);
closure *enclose(closure *code, frame *current_frame, frame *base_frame);


// handling lambda-lists and function arguments

operation *build_argument_chain(closure *lambda_list,
				closure *arg_list,
				operation *current);
operation *make_arg(closure *sym, closure *val, operation *current);
// TODO name conflict

// continuations
machine *stack_copy(machine *m);
frame *copy_frame(frame *fram);

// accessing the bits of functions

closure *fn_lambda_list(closure *fn);
operation *fn_instructions(closure *fn);
operation *instruction_list(closure *list, operation *doit);


// symbol fetching and setting
closure *looker_up(closure *sym, frame *current_frame, frame *base_frame);

void internal_set(closure *sym,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame);


// running the machine
frame * new_frame(frame *below);
void do_internal(closure *ins, machine *m);
int virtual_machine_step(machine *m);
machine *eval(closure *form, machine *m);

void toss_signal(closure* sig, machine* m);
closure * build_signal(closure *a, machine *m);


//----------------------- PRINT.C -----------------------//

void print_string(closure *a);
void print_cons(closure *cons);
void print_stack(frame *fm);
void print_closure(closure *x);
void print_cont(operation *ins);
void print_op(operation* ins);
void print_machine(machine *m);


//--------------------- STRINGS.C -----------------------//

closure *string(wchar_t * str);
int stringp(closure* a);
closure* character(wchar_t a);
wchar_t* string_to_c_MALLOC(closure *a);
closure *string_to_number(closure *a);

//------------------------ PARSE.C -------------------------//

closure *parse_file(FILE *file);

//----------------------- SYMBOL_TABLE.C -------------------//

symbol_table *new_symbol_table();

void table_insert(closure *symbol, closure *value, symbol_table *table);

closure *table_lookup(closure *symbol, symbol_table *table);

symbol_table *table_union(symbol_table *a, symbol_table *b);

//------------------ BASIC_FUNCTIONS.C -------------------//

void new_basic_commands(machine *m);

// basics
void is_fn(machine *m);
void quote_fn(machine *m);
void oif_fn(machine *m);
void cons_fn(machine *m);
void car_fn(machine *m);
void cdr_fn(machine *m);
void comma_fn(machine *m);
void callcc_fn(machine *m);

void atomp_fn(machine *m);
void leak_fn(machine *m);

// setting 
void set_fn(machine *m);
void set_car_fn(machine *m);
void set_cdr_fn(machine *m);

// signaling
void toss_signal(closure* sig, machine* m);

void signal_fn(machine *m);
void unhandle_signal(machine *m);
void add_handler(machine *m);     
void base_handler(machine *m);

//debugging
void print_fn(machine *m);
void prmachine_fn(machine *m);
void start_debug_fn(machine *m);
void stack_trace_fn(machine *m);
void globals_fn(machine *m);

//maths
void plus_fn(machine *m);     
void minus_fn(machine *m);   
void multiply_fn(machine *m);     
void divide_fn(machine *m);     
void greater_fn(machine *m);    
void less_fn(machine *m);

// getting additional info
void closing_of_fn(machine *m);
void set_info_fn(machine *m);
void get_info_fn(machine *m);

//file handle actions
void read_file_fn(machine *m);
void close_file_fn(machine *m);
void read_char_fn(machine *m);

// character tests
void whitespacep_fn(machine *m);
void eof_p_fn(machine *m);
void character_p_fn(machine *m);

// string manip
void string_to_symbol_fn(machine *m);
void symbol_to_string_fn(machine *m);
void string_to_number_fn(machine *m);


void symbol_table_fn(machine *m);
void insert_fn(machine *m);
void lookup_fn(machine *m);
void table_union_fn(machine *m);

void load_library_fn(machine *m);
void load_library_object_fn(machine *m);
void call_library_function_fn(machine *m);

void intern_builtin_functions(machine *m);


#endif
