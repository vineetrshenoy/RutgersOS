## Makefile

CC = gcc
CFLAGS = -Wall -g
OBJS = main.o my_pthread.o mymalloc.o

manager: $(OBJS)
	$(CC) -g -o manager $(OBJS) 

main.o: main.c
	$(CC) -c -g main.c 

my_pthread.o: my_pthread.c my_pthread_t.h
	$(CC) -c -g my_pthread.c 

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) -c -g mymalloc.c