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

struct itimerval timer;
my_pthread_t * tail;
int threadIDS = 1;
int queueSize = 0;
int isInitialized = 0;




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

	thread = (my_pthread_t *) malloc(sizeof(my_pthread_t)); //Malloc space for new thread
	thread->context = (ucontext_t *) malloc(sizeof(ucontext_t)); 	//Also malloc space for ucontext

	//Get context to initialize new thread
	if(getcontext(thread->context) == -1){
		printf("Error getting context. Returning -1\n");
		return -1;
	}

	ucontext_t * newContext = thread->context;
	(*newContext).uc_stack.ss_sp = malloc(STACK_SIZE);	//mallocs new stack space
	(*newContext).uc_stack.ss_size = STACK_SIZE;	//describes size of stack
	(*newContext).uc_link = NULL;
	

	thread->thread_id = threadIDS; //Gives the thread an ID
	makecontext(thread->context, function, 1, arg); //creates with function. Users usually pass a struct of arguments?
	enqueueRear(thread);	//Adds thread to priority queue

	//If this is the first time calling my_pthread_create()
	if (isInitialized == 0){
		isInitialized = 1;	//change isInitialized flag
		my_pthread_t * mainThread = (my_pthread_t *) malloc(sizeof(my_pthread_t)); //malloc space for the my_pthread struct
		mainThread->context = (ucontext_t *) malloc(sizeof(ucontext_t));	//malloc space for main contex
		mainThread->thread_id = 0;	//Zero will always be thread id for main
		getcontext(mainThread->context);	//Saves the current context of main
		enqueueRear(mainThread);	//Adds main the the priority queue

	}

	//my_pthread_yield();



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


/*
	This function dequeues an item from the front

	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure

*/

my_pthread_t * dequeueFront(){

	//If no elements exist in queue, set tail to new element, and have next point to itself. Increase size
	//Same as enqueueFront
	if (queueSize == 0){
		printf("No elements in queue. Returning NULL\n");
		return NULL;
	}
	else if (queueSize == 0) {
		queueSize--;
		return tail;
	}

	my_pthread_t * returnThread;
	returnThread = tail->next;
	tail->next = returnThread->next;
	returnThread->next = NULL;
	queueSize--;
	return returnThread;
	
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
	
	//void * (*functionPointer)(void *);
	//functionPointer = &printFunction;

	//void(*otherFunction)();
	//otherFunction = &printFunction;

	my_pthread_t * thread;

	my_pthread_create(thread, NULL, printFunction, NULL);	

	printf("The tail node is %d\n", tail->thread_id);
	printf("The next node is %d\n", tail->next->thread_id);

	my_pthread_t * node = dequeueFront();
	printf("Thread id is %d\n", node->thread_id);

	//ucontext_t * otherContext = node->context;
	//makecontext(otherContext, otherFunction,0);
	setcontext(node->context);	
	

	printf("Ending main\n");
	return 0;
}