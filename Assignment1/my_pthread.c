#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>

#define STACK_SIZE 1000




void printFunction(){
	printf("Entered printFunction ONE\n");
}



int main(){
	

	ucontext_t ucp, ucp_main;	//Creates a ucontext object
	
	//Gets the current context and stores it in ucp. Prints error on failure
	if (getcontext(&ucp) == -1)
		printf("Error retrieving context\n");
	ucp.uc_stack.ss_sp = malloc(STACK_SIZE);	//Allocate new stack space
	ucp.uc_stack.ss_size = STACK_SIZE;			//Specify size of stack
	ucp.uc_link = &ucp_main;							//Sets the link to null

	void (*functionPointer)();		//Create a function pointer to printFunction
	functionPointer = &printFunction;

	makecontext(&ucp, functionPointer, 0);	//assigns function printFunction in  ucp

	swapcontext(&ucp_main, &ucp);			//switches to printFunction, executes, returnts ot main
	

	printf("Ending main\n");
	return 0;
}