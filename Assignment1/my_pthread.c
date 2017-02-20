#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 100000


ucontext_t ucp, ucp_two, ucp_main;
volatile int x;
struct itimerval timer;

void printFunction(){
	
	
	printf("Waiting for signal handler\n");
	sleep(1);
	printf("We are here now \n");

	
	
}


void handler(int sig){
	printf("In signal handler\n");
	setcontext(&ucp_main);
}



int main(){
	
	struct itimerval timer;
	signal(SIGALRM,handler);	//Creates the signal handler

	//it_interval value to which reset occurs 
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 100000;
	
	//it_value is value that is counted down. Once zero, sends signal
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	
	

	if (getcontext(&ucp) == -1)
		printf("Error retrieving context\n");
	ucp.uc_stack.ss_sp = malloc(STACK_SIZE);	//Allocate new stack space
	ucp.uc_stack.ss_size = STACK_SIZE;			//Specify size of stack
	ucp.uc_link = NULL;



	void (*functionPointer)();
	functionPointer = &printFunction;
	makecontext(&ucp, functionPointer, 0);	//Creates the context
	setitimer(ITIMER_REAL, &timer, NULL);	//sets the itimer
	swapcontext(&ucp_main, &ucp);			//swaps to the other context
	
	x = 5;
	

	
	printf("In main ... the value of x is %d \n", x);
	

	
	


	printf("Ending main\n");
	return 0;
}