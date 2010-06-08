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

/*
This is intended to be very simple, because it is only there to parse
unlambda, fn, def, and the full parser. No numbers or error-checking
is included.
 */

#include <stdio.h>
#include "eight.h"
#include <ctype.h>

closure *parse_list(FILE *file, closure *accum);
closure *parse_string(FILE *file, closure *accum);
closure *parse_symbol(FILE *file, closure *accum);
closure *parse_null(FILE *file, closure *accum);
closure *parse_character(FILE *file, closure *accum);
void parse_comment(FILE *file, closure *accum);

closure *reverse(closure *a);

closure *reverse(closure *a){
    closure *ret = nil();
    while(!nilp(a)){
	ret = cons(car(a), ret);
	a = cdr(a);
    }
    return ret;
}


closure *parse_file(FILE *file)
{
    return parse_null(file, nil());
}

closure *parse_null(FILE *file, closure *accum)
{
    //printf("parse null\n");
    wchar_t c = fgetwc(file);
    if(iswspace(c)){
	return parse_null(file, nil());
    } else if(c == L'$'){
	return parse_character(file, nil());
    } else if(c == L'#'){
	parse_comment(file, nil());
	return parse_null(file, accum);
    } else if(c == L'\"'){
	return parse_string(file, nil());
    } else if(c == L'('){
	return parse_list(file, nil());
    } else if(c == WEOF || c == EOF){
	return NULL;
    } else {
	return parse_symbol(file, cons(character(c), nil()));
    }
}

closure *parse_symbol(FILE *file, closure *accum)
{
    //printf("parse sym\n");
    wchar_t c = fgetwc(file);
    if(iswspace(c)) {
	return string_to_symbol(reverse(accum));
    } else if(c == L'\"'){
	ungetwc(c, file); 
	return string_to_symbol(reverse(accum));
    } else if(c == L'('){
	ungetwc(c, file); 
	return string_to_symbol(reverse(accum));
    } else if(c == L')') {
	ungetwc(c, file); 
	return string_to_symbol(reverse(accum));
    } else if(c == L'#'){
	ungetwc(c, file); 
	return string_to_symbol(reverse(accum));
    } else {
	return parse_symbol(file, cons(character(c), accum));
    }
}

closure *parse_string(FILE *file, closure *accum)
{
    //printf("parse str\n");
    wchar_t c = fgetwc(file);
    if(c == L'\"'){
	return reverse(accum);
    } else {
	return parse_string(file, cons(character(c), accum));
    }
}

closure *parse_character(FILE *file, closure *accum)
{
    //printf("parse char\n");
    wchar_t c = fgetwc(file);
    return character(c);
}

closure *parse_list(FILE *file, closure *accum)
{
    //printf("parse list\n");
    wchar_t c = fgetwc(file);
    if(iswspace(c)){
	return parse_list(file, accum);
    } else if(c == L'$'){
	return parse_list(file, cons(parse_character(file, nil()), accum));
    } else if(c == L'\"'){
	return parse_list(file, cons(parse_string(file, nil()), accum));
    }else if(c == L'('){
	return parse_list(file, cons(parse_list(file, nil()), accum));
    } else if(c == L'#'){
	parse_comment(file, nil());
	return parse_list(file, accum);
    } else if(c == L')'){
	return reverse(accum);
    } else {
	return parse_list(file, cons(
				     parse_symbol(file,
						  cons(character(c), nil())), 
				     accum));
    }
}


void parse_comment(FILE *file, closure *accum)
{
    wchar_t c = fgetwc(file);
    while(c != L'\n'){
	c = fgetwc(file);
    }
}
