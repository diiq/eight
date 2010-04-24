SHELL = /bin/sh

# WARNING you'll need the boehm garbage collector, byacc, and flex in 
# order to compile.

all:
	yacc -d yacc.yacc; flex lexer.lex
	gcc lex.yy.c -I/usr/include/gc/ -lm -o eight -g