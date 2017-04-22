## Makefile

CC = gcc
CFLAGS = -Wall -g
OBJS = block.o test.o

manager: $(OBJS)
	$(CC) -g -o manager $(OBJS) -lm

block.o: block.c block.h
	$(CC) -c -g block.c 

test.o: test.c test.h
	$(CC) -c -g test.c 