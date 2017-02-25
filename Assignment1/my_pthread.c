// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: 

//test for commit

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 100000

typedef enum {
	ACTIVE,
	WAITING,
	COMPLETED
} my_pthread_state;

typedef struct my_pthread_t {

	int thread_id;
	ucontext_t * context;
	char * string;
	struct my_pthread_t * next;
	my_pthread_state state;
	void* return_value;

}my_pthread_t;

typedef struct{} my_pthread_attr_t;

typedef struct {
	int mutex_id;
} my_pthread_mutex_t;

typedef struct{} my_pthread_mutexattr_t;

typedef struct my_pthread_mutex_node 
{
	my_pthread_mutex_t *mutex;
	my_pthread_t *holder;
} my_pthread_mutex_node;

ucontext_t ucp, ucp_two, ucp_main;

struct itimerval timer;
my_pthread_t * tail;
int threadIDS = 1;
int queueSize = 0;
int isInitialized = 0;
int totalThreads = 0;
int current_thread_id = 1;




int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t * attr, void * (*function)(void*), void* arg){

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

	// something like this should go here to add the main function to the top of the queue:
	//if queuesize == 0:
	// getcontext(main)
	// add this context to a thread instance
	// increment totalThreads for thread_id
	//add to queue



	// add context to thread
	ucontext_t* b = (ucontext_t*) malloc(sizeof(ucontext_t*));
	thread->context = b;
	getcontext(thread->context);
	thread->context->uc_stack.ss_sp = malloc(STACK_SIZE);
	thread->context->uc_stack.ss_size = STACK_SIZE;
	thread->state = ACTIVE;
	thread->thread_id = totalThreads++;
	// add function to context
	makecontext(thread->context, (void*) function, arg);
	// add to queue


	return 0;


}


void my_pthread_yield(){

	/*
		This contains schedule code
	*/

	// check if current thread is done doing stuff
	// if not, add it to a lower priority queue
	my_pthread_t* current_thread = dequeueFront(); // of ready queue
	if (current_thread->state != COMPLETED) {
		enqueueFront(current_thread); // to lower priority queue
	}
	my_pthread_t* next_thread = dequeueFront(); //of ready queue
	enqueueFront(next_thread); // just to keep it on the queue as the current thing

	// just in case the current thread is also the next thread
	if (current_thread_id == next_thread->thread_id) {
			return;
	}

	current_thread_id = next_thread->thread_id;
	setcontext(next_thread->context);

	// setitimer stuff

	timer.it_interval.tv_usec = 50000;
	timer.it_value.tv_usec = 50000;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	return;


}


void my_pthread_exit(void * value_ptr){

	/*
	ATOMIC OPERATION
	1. Assign the return value of the function to value_ptr
	2. Deallocate the stack
	3. Remove context from the priority queue
	4. my_pthread_yield
 
	*/

	// get completed thread and set return value and state
	my_pthread_t* current_thread = dequeueFront();
	current_thread->return_value = value_ptr;
	current_thread->state = COMPLETED;

	// put it back on ready queue so yield can take it off
	enqueueFront(current_thread);

	my_pthread_yield();

	return;


}


int my_pthread_join(my_pthread_t thread, void ** value_ptr){
	

	/*
	1. Calling thread will not run until called thread is done running
	2. Call yield()
	
	*/

	// not sure how to do this but it should be something like this:
	// get current thread (calling thread)
	my_pthread_t* current_thread = dequeueFront();
	if (thread->state != COMPLETED) {
		current_thread->state = WAITING;
		enqueueRear(current_thread); // to waiting queue
		enqueueFront(current_thread); // to ready queue so yield can take it off
	}

	// keep running scheduler until target thread has been completed
	while (thread->state != COMPLETED) {
		my_pthread_yield();
	}

	current_thread->state = ACTIVE;
	enqueueFront(current_thread); // to ready queue

	if (value_ptr != NULL) {
		return thread->return_value; // return target thread value
	}

	return 0;

}




int my_pthread_mutex_init(my_pthread_mutex_t * mutex, const my_pthread_mutexattr_t * mutexattr){

	// mutex queue is needed
	// new mutex node added to mutex queue
	// return 0 for success, -1 for failure

}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

	// dequeFront() of mutex queue
	// mutex node needs "holder" characteristic
	// holder = currentthread
	// return 0, -1 for failure
}

 int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){

	// get front mutex node from queue
	// if current thread == holder of node
	// return 0, -1 for failure

 }

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){

	// my_pthread_mutex_unlock(*mutex) to make sure
	// deallocate stuff
	// return 0, -1 for failure

}


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
