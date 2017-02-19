#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 1000


ucontext_t ucp, ucp_two, ucp_main;

void printFunction(){
	printf("Entered printFunction ONE\n");
	
}


void printFunctionTwo(){
	printf("Entered printFunction TWO \n");
}


void handler(int sig){
	printf("Entered the signal handler...now exiting\n");
	exit(1);
}

int main(){
	struct itimerval timer;
	signal(SIGVTALRM,handler);

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 100000;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	
	setitimer(ITIMER_VIRTUAL, &timer, NULL);


	while(1){

	}


	printf("Ending main\n");
	return 0;
}