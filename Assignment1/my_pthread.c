#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 100000

typedef struct my_pthread_t {

	int thread_id;
	ucontext_t * context;
	char * string;
	struct my_pthread_t * next;

}my_pthread_t;

ucontext_t ucp, ucp_two, ucp_main;
volatile int x;
struct itimerval timer;
my_pthread_t * tail;
int queueSize = 0;




int my_pthread_create(my_pthread_t *thread, pthread_attr_t * attr, void * (*function)(void*), void* arg){

	/*

	First time around,this function returns two contexts.


		1. Create a new context
		2. Allocate stack space
		3. Stack size
		4. uc_link 



		Have some global variable that checks if my_pthread_create has not been run before. If not,
		1. Initialize priority queues
		2. Create a context for the main
		3. Add to the ready queue
		4. Only keep one version of main use version on the queue
	*/




}


//void my_pthread_yield(){

	/*
		This contains schedule code


	*/

//}


//void my_pthread_exit(void * value_ptr){

	/*
	ATOMIC OPERATION

	1. Assign the return value of the function to value_ptr
	2. Deallocate the stack
	3. Remove context from the priority queue
	4. my_pthread_yield

 

	*/


//}


//int my_pthread_join(my_pthread_t thread, void ** value_ptr){
	

	/*
	1. Calling thread will not run until called thread is done running
	2. Call yield()
	
	*/


//}




//int my_thread_mutex_init(my_pthread_t * mutex, const pthread_mutexattr_t * mutexattr){

//}




/*
	This function inputs a new thread at the beginning of the queue

	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure

*/

int enqueueFront(my_pthread_t * newThread){

	//If no elements exist in queue, set tail to new element, and have next point to itself. Increase size
	if (queueSize == 0){
		tail = newThread;
		tail->next = tail;
		queueSize++;
		return 1;
	}


	//If one element or greater exists
	newThread->next = tail->next;	//new thread points to front (become front)
	tail->next = newThread;			//next of tail points to new thread
	queueSize++;
	return 1;
}


/*
	This function inputs a new thread at the rear of the queue

	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure

*/

int enqueueRear(my_pthread_t * newThread){

	//If no elements exist in queue, set tail to new element, and have next point to itself. Increase size
	//Same as enqueueFront
	if (queueSize == 0){
		tail = newThread;
		tail->next = tail;
		queueSize++;
		return 1;
	}


	//If one element or greater exists
	newThread->next = tail->next;	//the new thread points to the first element
	tail->next = newThread;			//Old tail points to the new tail
	tail = tail->next;			//Tail variable moves to new tail
	queueSize++;
	return 1;
}







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
	/*
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
	

	*/
	/*
	my_pthread_t * first = (my_pthread_t *) malloc (sizeof(my_pthread_t));
	my_pthread_t * second = (my_pthread_t *) malloc (sizeof(my_pthread_t));
	head = first;
	first->context = "This is the first node\n";
	first->next = second;
	second->context = "This is the second node\n";
	second->next = NULL;

	

	printf("%s\n", first->context);
	printf("%s\n", first->next->context);
	*/
	


	printf("Ending main\n");
	return 0;
}