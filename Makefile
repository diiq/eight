SHELL = /bin/sh

# WARNING you'll need the boehm garbage collector, byacc, and flex in 
# order to compile.

all:
	byacc yacc.yacc; flex lexer.lex
	gcc lex.yy.c -I/usr/include/gc/ -lm -lgc -o eight