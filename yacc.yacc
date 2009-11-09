%{
#include "stdio.h"
#include "eight.h"
#include "eight.c"
#include "print.c"
#include "basic_commands.c"
#include "symbols.c"
#include "strings.c"
  machine *m;
%}

%start root

%token END
%token OPEN
%token CLOSE
%token STRIN
%token NUMBE
%token SYMBO
%token QUOT
%token ASTERI
%token ELIPSI
%token COMM

%%

root:
    | list                   { //print_closure((closure*)$1);
                              return (int)eval((closure *)$1, m); } 
    | atom                   { //print_closure((closure*)$1);
                              return (int)eval((closure *)$1, m); }  
    | END                    { return -1; }

list:
    OPEN cons CLOSE          { $$ = (int)$2; }
    | OPEN CLOSE             { $$ = (int)nil();}
    | QUOT list              { $$ = (int)cons(symbol(QUOTE), 
					 cons((closure *)$2, nil())); }
    | ASTERI list            { $$ = (int)cons(symbol(ASTERIX), 
					 cons((closure *)$2, nil())); }
    | COMM list              { $$ = (int)cons(symbol(COMMA), 
					 cons((closure *)$2, nil())); }

cons: 
    atom cons                { $$ = (int)cons((closure *)$1, (closure *)$2);} 
    | list cons              { $$ = (int)cons((closure *)$1, (closure *)$2);} 
    | list                   { $$ = (int)cons((closure *)$1, nil()); }
    | atom                   { $$ =(int) cons((closure *)$1, nil()); }

    
atom:
    QUOT atom                { $$ =  (int)cons(symbol(QUOTE), 
					  cons((closure *)$2, nil())); }
    | ASTERI atom            { $$ =  (int)cons(symbol(ASTERIX), 
					  cons((closure *)$2, nil())); }
    | COMM atom              { $$ =  (int)cons(symbol(COMMA), 
					  cons((closure *)$2, nil())); }
    | NUMBE                  { $$ = (int)fixnum($1); }
    | SYMBO                  { $$ = (int)symbol(string_to_symbol_id((char*)$1)); }
    | ELIPSI                 { $$ = (int)symbol(ELIPSIS); }
    | STRIN                  { $$ = (int)string((char*)$1); }


%%
void set_input_source(FILE *s);

int main( int   argc,
          char *argv[] )
{
     GC_INIT();
     initialize_symbol_table();
     m = init_8VM();
     set_input_source(fopen( "eight.8", "r" ));
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
