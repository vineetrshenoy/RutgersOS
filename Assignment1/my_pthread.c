#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>

#define STACK_SIZE 1000


ucontext_t ucp, ucp_two, ucp_main;

void printFunction(){
	printf("Entered printFunction ONE\n");
	swapcontext(&ucp, &ucp_two);
}


void printFunctionTwo(){
	printf("Entered printFunction TWO \n");
}


int main(){
	
	/*
	This function does the following
	1. Sets all values in main (see below). Sends execution to printFunction
	2. Prints statement, sends execution to printFunctionTwo
	3. Prints statement, send execution back to main using uc_link (ucp_main context
	was stored on swapcontext)
	4. Main print "Ending main"


	*/

		
	
	//Gets the current context and stores it in ucp. Prints error on failure
	//For printFunction
	if (getcontext(&ucp) == -1)
		printf("Error retrieving context\n");
	ucp.uc_stack.ss_sp = malloc(STACK_SIZE);	//Allocate new stack space
	ucp.uc_stack.ss_size = STACK_SIZE;			//Specify size of stack
	ucp.uc_link = &ucp_two;							//Sets the link to null


	//for printFunctionTwo
	if (getcontext(&ucp_two) == -1)
		printf("Error retrieving context\n");
	ucp_two.uc_stack.ss_sp = malloc(STACK_SIZE);	//Allocate new stack space
	ucp_two.uc_stack.ss_size = STACK_SIZE;			//Specify size of stack
	ucp_two.uc_link = &ucp_main;							//Sets the link to null

	
	void (*functionPointer)();		//Create a function pointer to printFunction
	functionPointer = &printFunction;


	void (*functionPointerTwo)();		//Create a function pointer to printFunctionTwo
	functionPointerTwo = &printFunctionTwo;

	makecontext(&ucp, functionPointer, 0);	//assigns function printFunction in  ucp
	makecontext(&ucp_two, functionPointerTwo,0); //assigns function printFunctionTwo in  ucp_two

	//Procedure: main -> printFunction -> printFunctionTwo -> main
	swapcontext(&ucp_main, &ucp);			
	

	printf("Ending main\n");
	return 0;
}