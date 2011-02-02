SHELL = /bin/sh

all:
	gcc main.c -o eight -O3 

test:
	gcc main.c -o eight -O3 
	./eight test.8

debug:
	gcc main.c -o eight -O3 -ldl -g


profile:
	gcc main.c -o eight -O3 -ldl -pg
	./eight test.8 test.8
	gprof ./eight > profile_output


coverage:
	gcc main.c -o eight -O3	 -fprofile-arcs -ftest-coverage
