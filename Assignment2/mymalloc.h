#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>	
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>





void test();
void memvoid();
void * getHeader(void * p);
void * getFooter(void * p);
int getSize(void * headerPointer);
int getAllocation(void * p);
char * createExtremities(char * p, int size, int allocated);
void mydeallocate(void * ptr, char * b, int a, int id);
void setValue(void * p, int size, int allocation);
void * getNext(void * ptr);
void * getPrevious(void * ptr);
void initialize();
void * findFit(int extendedSize);
void * myallocate(size_t size, char * b, int a, int id);
void coalesce(void * ptr);
void initializeMemory();
void initializePage();
void swap_in(int16_t newPage, int des);
int16_t swap_out(int16_t page);
static void seg_handler(int sig, siginfo_t * si, void * unused);
void install_seg_handler();


#endif