#include "stdio.h"
#include "eight.h"
#include "eight.c"
#include "print.c"
#include "basic_commands.c"
#include "symbols.c"
#include "strings.c"
#include "memory.c"
#include "parsing.c"

void interact_parse(machine *m)
{
    FILE *file = stdin;
    printf("-> ");
    closure *rep = parse_file(file);
    while (rep != NULL){
	m = eval(rep, m);
	if (m->accum != NULL){
	    print_closure(m->accum);
	    printf("\n");
	    printf("-> ");
	}
	rep = parse_file(file);
    }
}

int main( int   argc,
          char *argv[] )
{
     initialize_symbol_table();
     machine *m = init_8VM();
     FILE *file = fopen("floor.8", "r");
     closure *rep = parse_file(file);
     while (rep != NULL){
	 m = eval(rep, m);
	 rep = parse_file(file);
     }
     fclose(file);
     int i;
     if(argc == 1){
	 interact_parse(m);
     } else {
	 for (i=1; i<argc; i++){
	     if(!strcmp(argv[i], "-i")){
		 interact_parse(m);
	     } else {
		 rep = parse_file(fopen(argv[i], "r"));
		 while (rep != NULL){
		     m = eval(rep, m);
		     rep = parse_file(file);
		 }
	     }
	 }
     }
     return 0; 
}
