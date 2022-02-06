/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley

***************************************************************************/
#ifndef EIGHT_PARSE
#define EIGHT_PARSE


/*
This is intended to be very simple, because it is only there to parse
unlambda, fn, def, and the full parser. No error-checking
is included.
 */

#include <stdio.h>
#include "eight.h"
#include <ctype.h>

closure *parse_list(FILE *file, closure *accum);
closure *parse_string(FILE *file, closure *accum);
closure *parse_symbol(FILE *file, closure *accum);
closure *parse_number(FILE *file, closure *accum);
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
    closure *boo = parse_null(file, nil());
    return boo;
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
    } else if(c == L'\''){
	wchar_t c = fgetwc(file);
	if (!iswspace(c)){
	    ungetwc(c, file);
	    closure *boo = parse_null(file, nil());
	    if (boo != NULL) return quote(boo);
	}
	return symbol(QUOTE);
    } else if(c == L'@'){
	wchar_t c = fgetwc(file);
	if (!iswspace(c)){
	    ungetwc(c, file);
	    closure *boo = parse_null(file, nil());
	    if (boo != NULL) return list(2, symbol(ATPEND), boo);
	}
	return symbol(ATPEND);
    } else if(c == L','){
	wchar_t c = fgetwc(file);
	if (!iswspace(c)){
	    ungetwc(c, file);
	    closure *boo = parse_null(file, nil());
	    if (boo != NULL) return list(2, symbol(COMMA), boo);
	}
	return symbol(COMMA);
    } else if(c == L'*'){
	wchar_t c = fgetwc(file);
	if (!iswspace(c)){
	    ungetwc(c, file);
	    closure *boo = parse_null(file, nil());
	    if (boo != NULL) return list(2, symbol(ASTERIX), boo);
	}
	return symbol(ASTERIX);
    } else if(iswdigit(c)){
	ungetwc(c, file);
	return parse_number(file, nil());
    } else if(c == L'('){
	return parse_list(file, nil());
    } else if(c == L')'){
	ungetwc(c, file);
	return NULL;
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

closure *parse_number(FILE *file, closure *accum)
{
    wchar_t c = fgetwc(file);
    if(iswdigit(c)){
	return parse_number(file, cons(character(c), accum));
    } else if(iswspace(c)) {
	return string_to_number(reverse(accum));
    } else if(c == L')') {
	ungetwc(c, file); 
	return string_to_number(reverse(accum));
    } else {
	ungetwc(c, file);
	return parse_symbol(file, accum);
    }
}

closure *parse_string(FILE *file, closure *accum)
{
    //printf("parse str\n");
    wchar_t c = fgetwc(file);
    if(c == L'\"'){
	return reverse(accum);
    } else if (c == '\\') {
	wchar_t c = fgetwc(file);
	return parse_string(file, cons(character(c), accum));
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
    //print_closure(accum);
    wchar_t c = fgetwc(file);
    if(c == L')'){
	return reverse(accum);
    } else {
	ungetwc(c, file); 
	closure *boo = parse_null(file, nil());
	if (boo != NULL){
	    return parse_list(file, cons(boo, accum));
	} else {
	    return parse_list(file, accum);
	}
    }
}


void parse_comment(FILE *file, closure *accum)
{
    wchar_t c = fgetwc(file);
    while(c != L'\n'){
	c = fgetwc(file);
    }
}

#endif
