## Makefile

CC = gcc
CFLAGS = -Wall -g
OBJS = my_pthread.o mymalloc.o

manager: $(OBJS)
	$(CC) -g -o manager $(OBJS) 

my_pthread.o: my_pthread.c my_pthread_t.h
	$(CC) -c -g my_pthread.c 

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) -c -g mymalloc.c