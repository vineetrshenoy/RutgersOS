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

ucontext_t ucp, ucp_two, ucp_main;
volatile int x;
struct itimerval timer;
my_pthread_t * tail;
int queueSize = 0;
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

	

}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

	if (*mutex.__m_lock.__status != 0) {
		my_pthread_yield();
	}
	else {
		*mutex.__m_lock.__status == 1;
	}
	return 0;

}

 int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){

	*mutex.__m_lock.__status == 0;
	return 0;

 }

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){

	*mutex = NULL;
	return 0;

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
	my_pthread_t * counter;

	// The first thread
	my_pthread_t * threadOne = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadOne->string = "This is the first thread";
	enqueueFront(threadOne);

	//The second thread
	my_pthread_t * threadTwo = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadTwo->string = "This is the second thread";
	enqueueFront(threadTwo);

	//The second thread
	my_pthread_t * threadThree = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadThree->string = "This is the third thread";
	enqueueFront(threadThree);

	printf("%s and queueSize %d\n", tail->string, queueSize);
	counter = tail->next;
	while (counter != tail){
		printf("%s and queueSize %d\n", counter->string, queueSize);
		counter = counter->next;
	}


	my_pthread_t * value = dequeueFront();
	printf("%s and queue size %d\n", value->string, queueSize);

	my_pthread_t * valuetwo= dequeueFront();
	printf("%s and queue size %d\n", valuetwo->string, queueSize);

	my_pthread_t * valuethree = dequeueFront();
	printf("%s and queue size %d\n", valuethree->string, queueSize);

	my_pthread_t * nullValue = dequeueFront();
	

	printf("Ending main\n");
	return 0;
}
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
	my_pthread_t * counter;

	// The first thread
	my_pthread_t * threadOne = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadOne->string = "This is the first thread";
	enqueueFront(threadOne);

	//The second thread
	my_pthread_t * threadTwo = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadTwo->string = "This is the second thread";
	enqueueFront(threadTwo);

	//The second thread
	my_pthread_t * threadThree = (my_pthread_t *) malloc(sizeof(my_pthread_t));
	threadThree->string = "This is the third thread";
	enqueueFront(threadThree);

	printf("%s and queueSize %d\n", tail->string, queueSize);
	counter = tail->next;
	while (counter != tail){
		printf("%s and queueSize %d\n", counter->string, queueSize);
		counter = counter->next;
	}


	my_pthread_t * value = dequeueFront();
	printf("%s and queue size %d\n", value->string, queueSize);

	my_pthread_t * valuetwo= dequeueFront();
	printf("%s and queue size %d\n", valuetwo->string, queueSize);

	my_pthread_t * valuethree = dequeueFront();
	printf("%s and queue size %d\n", valuethree->string, queueSize);

	my_pthread_t * nullValue = dequeueFront();
	

	printf("Ending main\n");
	return 0;
}
