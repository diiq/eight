#ifndef EIGHT_HEADER
#define EIGHT_HEADER

#define new(x) (x *)allocate(sizeof(x));
//#define new(x) (x *)calloc(1, sizeof(x));

int DEBUG = 0;

typedef struct closure_struct closure;
typedef int symbol_id;
typedef struct operation_struct operation;
typedef struct frame_struct frame;

typedef enum  {
  EMPTY, REFERENCE, // for garbage collecting
  NIL, CONS_PAIR, NUMBER, INTERNAL, CHARACTER, SYMBOL, CONTINUATION, 
  BUILTIN, C_OBJECT,  
  MACHINE_FLAG, CLOSURE_OP, /// these are operation types
  MACHINE, FRAME, CONS_CELL /// etc
} obj_type;

typedef enum {
  DO, APPLY, CONTINUE_APPLY, ATPEND_APPLY, ARGUMENT, E_ARGUMENT, SIGNAL
} machine_flag;

typedef enum {
     ELIPSIS, ASTERIX, COMMA, QUOTE, ATPEND, LEAKED, T, CLEAR
} special; 

 
struct operation_struct{
     obj_type type;
     machine_flag flag;
     closure *closure;
     operation *next;
};

struct frame_struct {
     obj_type type;
     operation     *next; 
     closure     *rib;
     closure     *scope;
     closure     *signal_handler; 
     frame     *below;
};

typedef struct {
     obj_type type;
     frame *current_frame;
     frame *base_frame;
     closure *accum;
} machine;

typedef void (* builtin) (machine*);

typedef struct {
     obj_type type;
     closure *car;
     closure *cdr;
} cons_cell;

struct closure_struct {
     obj_type    type;
     closure *closing;
     closure *info;
     union {
	  void    *builtin_fn;
	  void    *c_object;
	  machine *mach;
	  symbol_id symbol_id;
	  char    character;
	  int     num;
	  cons_cell *cons;
	  void    *obj;
     };
};


typedef struct memory_block_struct memory_block;

typedef struct {
     memory_block *block;
     int offset;
} memory_location;

typedef struct {
     obj_type type;
     void *point;
     int size;
} memory_reference;

typedef struct {
     memory_block *first;
     memory_location *current;
     memory_location *from;
     int size; // this is in units of BLOCK_SIZE
} memory;


struct memory_block_struct {
     void *this;
     memory_block *next;
};





machine *collect();
void *repair_reference(void *ref);
void copy_memory(int start, int end);
void collectify();
void *allocatery(memory *mo, int size);
void free_memory(memory *a);
machine *collect();
int mysizeof(void *typed);
void print_type(obj_type type);
void test_memory();

// memory allocation
machine * init_memory();
void *allocate(int size);
//void garbage_collect();

//symbol table
void initialize_symbol_table();
void insert_symbol(char *name, int val);
symbol_id string_to_symbol_id(char *name);
char* symbol_id_to_string(symbol_id sym);

// making symbols
closure *nil();
int      nilp(closure *x);
closure *symbol(symbol_id id);
closure *quote(closure *x);
closure *number(int num);

// symbol fetching and setting
closure *looker_up(closure *sym, frame *aframe);
closure *looker_up_internal(closure *sym, frame *aframe);
void internal_set(closure *sym,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame);

// the closure algebra
int equal(closure *a, closure *b);
closure *cheap_cons(closure *car, closure *cdr);
closure *copy_closure(closure *x);
closure *cons(closure *car, closure *cdr);
void combine(closure *a, 
	     closure *b, 
	     closure **newa, 
	     closure **newb,
	     closure **ret);
closure *cheap_car(closure *x);
closure *cheap_cdr(closure *x);
closure *rectify_closing_i(closure *closed, closure *closing, closure *ret);
closure *car(closure *x);
closure *cdr(closure *x);
closure *second(closure* list);
closure *rectify_closing(closure *closed);
closure *cheap_acons(closure *sym, closure *val, closure *closing);
closure *assoc(closure *sym, closure *closing);
closure *append(closure *a, closure *b);

// closings
int free_varp(closure *token,
	      closure *accum,
	      frame *current_frame);
closure *find_free_variables(closure *code,
			     frame* current_frame,
			     closure *accum);
closure *enclose(closure *code, frame *current_frame);

// handling lambda-lists and function arguments
int commap(closure *arg);
int asterixp(closure *arg);
int quotep(closure *sym);
int elipsisp(closure *sym);
int optional_argp(closure *sym);
int e_argp(closure *sym);
operation *build_argument_chain(closure *lambda_list,
				closure *arg_list,
				operation *current);
operation *make_arg(closure *sym, closure *val, operation *current);


// continuations
machine *stack_copy(machine *m);
frame *copy_frame(frame *fram);

// accessing the bits of functions
closure *fn_lambda_list(closure *fn);
operation *fn_instructions(closure *fn);
operation *instruction_list(closure *list, operation *doit);

// running the machine
frame * new_frame(frame *below);
void do_internal(closure *ins, machine *m);
int virtual_machine_step(machine *m);
machine *eval(closure *form, machine *m);

void print_heap(memory *m);
void print_string(closure *a);
void print_cons(closure *cons);
void print_stack(frame *fm);
void print_closure(closure *x);
void print_cont(operation *ins);
void print_op(operation* ins);
void print_machine(machine *m);
void new_basic_commands(machine *m);


void toss_signal(closure* sig, machine* m);
closure * build_signal(closure *a, machine *m);
closure *string(char * str);
// builtins
/*
read
pr
print
force
leak
car
cons
cdr
plus, minus, multiply, divide, <, >, (sin, cos, tan, asin, acos, atan, mod)
native, to-c-string, to-c-num, defcstruct, from-c-string, from-c-num 
set!
eq
oif
atomp (type?)
call/cc
signal
handle-signals
base-handler
*/
#endif
