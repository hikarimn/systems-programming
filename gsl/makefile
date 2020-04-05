CC=gcc
CFLAGS=-I/usr/include/gsl
LPATH=-L/usr/lib/x86_64-linux-gnu
LIBS=-lgsl

prog: solver.c
	$(CC) -o $@ $^ $(CFLAGS) $(LPATH) $(LIBS)
