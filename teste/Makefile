#
# Makefile para os testes
#

CC=gcc
CFLAGS=-Wall -I../include -L../lib -lcthread
LIB=../lib/libcthread.a

all: teste1 teste3

teste1: teste1.c $(LIB)
	$(CC) -o teste1 teste1.c $(CFLAGS)

teste3: teste3.c $(LIB)
	$(CC) -o teste3 teste3.c $(CFLAGS)

clean:
	rm -rf teste1 teste3  *.o *~
