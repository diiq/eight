%{
#include "stdio.h"
#include "eight.h"
#include "eight.c"
#include "print.c"
#include "basic_commands.c"
#include "symbols.c"
#include "strings.c"
#include "memory.c"

machine *m;

%}

%start root

%union {closure * clos;
  char * strin;
  int fixi;
};

%token END
%token OPEN
%token CLOSE
%token <strin> STRIN
%token <fixi> NUMBE
%token <strin> SYMBO
%token QUOT
%token ASTERI
%token ATPEN
%token ELIPSI
%token COMM

%type <clos> list cons atom


%%

root:
| list                   { print_closure((closure*)$1);	
                           printf("\n");
			   m = eval($1, m); return 1;} 
| atom                   { //print_closure((closure*)$1);
  m = eval($1, m); return 1;}  
    | END                    { return -1; }

list:
    OPEN cons CLOSE          { $$ = $2; }
    | OPEN CLOSE             { $$ = nil();}
    | QUOT list              { $$ = cons(symbol(QUOTE), 
					 cons($2, nil())); }
    | ASTERI list            { $$ = cons(symbol(ASTERIX), 
					 cons($2, nil())); }
    | ATPEN list            { $$ = cons(symbol(ATPEND), 
					 cons($2, nil())); }
    | COMM list              { $$ = cons(symbol(COMMA), 
					 cons($2, nil())); }

cons: 
    atom cons                { $$ = cons($1, $2);} 
    | list cons              { $$ = cons($1, $2);} 
    | list                   { $$ = cons($1, nil()); }
    | atom                   { $$ = cons($1, nil()); }

    
atom:
    QUOT atom                { $$ =  quote($2); }
    | ASTERI atom            { $$ =  cons(symbol(ASTERIX), 
					  cons($2, nil())); }
    | ATPEN atom            { $$ =  cons(symbol(ATPEND), 
					  cons($2, nil())); }
    | COMM atom              { $$ =  cons(symbol(COMMA), 
					  cons($2, nil())); }
    | NUMBE                  { $$ = number($1); }
    | SYMBO                  { $$ = symbol(string_to_symbol_id($1)); }
    | ELIPSI                 { $$ = symbol(ELIPSIS); }
    | STRIN                  { $$ = string($1); }


%%
void set_input_source(FILE *s);

int main( int   argc,
          char *argv[] )
{
     initialize_symbol_table();
     m = init_8VM();
     set_input_source(fopen( "test.8", "r" ));
      int rep = 0;
      while (rep != -1){
           rep = yyparse();
      }
      fclose(yyin);
      int i;
      int interact = 0;

      if(argc == 1){
	yyrestart(stdin);
	  interact = 1;
	  printf("-> ");
	  rep = 0;
	  while (rep != -1){
	    rep = yyparse();
	    if ((interact!=0) && (m->accum != NULL)){
	      print_closure(m->accum);
	      printf("\n");
	      printf("-> ");
	    }
	  }
      } 



      for (i=1;i<argc; i++){
	if (!strcmp(argv[i], "-i")){
	  yyrestart(stdin);
	  interact = 1;
	  printf("-> ");
	} else {
	  yyrestart(fopen(argv[i], "r"));
	  interact = 0;
	}
	  rep = 0;
	  while (rep != -1){
	    rep = yyparse();
	    if ((interact!=0) && (m->accum != NULL)){
	      print_closure(m->accum);
	      printf("\n");
	      printf("-> ");
	    }
	  }
	  fclose(yyin); 
	}
      return 0;
}

yyerror(s) char *s; {
     fprintf( stderr, "%s\n", s );
     return 1;
}
