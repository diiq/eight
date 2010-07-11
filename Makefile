SHELL = /bin/sh

all:
	gcc main.c -o eight -O3 

debug:
	gcc main.c -o eight -O3 -ldl -g

profile:
	gcc main.c -o eight -O3 -ldl -pg
	./eight test.8
	gprof ./eight > profile_output

