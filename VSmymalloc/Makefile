## Makefile

CC = gcc
CFLAGS = -Wall -g
OBJS = memgrind.o mymalloc.o

memgrind: $(OBJS)
	$(CC) -g -o memgrind $(OBJS) 

memgrind.o: memgrind.c mymalloc.h
	$(CC) -c -g memgrind.c 

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) -c -g mymalloc.c