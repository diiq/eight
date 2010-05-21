#include "stdio.h"
#include "eight.h"
#include "eight.c"
#include "print.c"
#include "basic_commands.c"
#include "symbols.c"
#include "strings.c"
#include "memory.c"
#include "parsing.c"


int main( int   argc,
          char *argv[] )
{
     initialize_symbol_table();
     machine *m = init_8VM();
     FILE *file = fopen("floor.8", "r");
     closure *rep = parse_file(file);
     while (rep != NULL){
	 print_closure(rep);
	 m = eval(rep, m);
	 rep = parse_file(file);
	 printf("\n");
     }
     fclose(file);
     int i;
     int interact = 0;

     if(argc == 1){
	 file = stdin;
	 interact = 1;
	 printf("-> ");
	 rep = parse_file(file);
	 while (rep != NULL){
	     m = eval(rep, m);
	     if ((interact!=0) && (m->accum != NULL)){
		 print_closure(m->accum);
		 printf("\n");
		 printf("-> ");
	     }
	     rep = parse_file(file);
	 }
	 }


     /*  for (i=1;i<argc; i++){ */
     /* 	if (!strcmp(argv[i], "-i")){ */
     /* 	  yyrestart(stdin); */
     /* 	  interact = 1; */
     /* 	  printf("-> "); */
     /* 	} else { */
     /* 	  yyrestart(fopen(argv[i], "r")); */
     /* 	  interact = 0; */
     /* 	} */
     /* 	  rep = 0; */
     /* 	  while (rep != -1){ */
     /* 	    rep = yyparse(); */
     /* 	    if ((interact!=0) && (m->accum != NULL)){ */
     /* 	      print_closure(m->accum); */
     /* 	      printf("\n"); */
     /* 	      printf("-> "); */
     /* 	    } */
     /* 	  } */
     /* 	  fclose(yyin);  */
     /* 	} */
     return 0; 
}
