// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: 

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include "my_pthread_t.h"

#define STACK_SIZE 100000

ucontext_t ucp, ucp_two, ucp_main;

struct itimerval timer;
queue_node * tail;
int threadIDS = 1;
int queueSize = 0;
int isInitialized = 0;
int totalThreads = 0;
int current_thread_id = 1;


int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t * attr, void * (*function)(void*), void* arg){

	

	// ------VINEET'S CODE ----
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
	threadIDS++;
	thread->state = ACTIVE;	//Sets thread to active stat
	

	makecontext(thread->context, (void (*)()) function, 1, arg); //creates with function. Users usually pass a struct of arguments?
	tail = enqueue(thread, tail, 0);	//Adds thread to priority queue

	//If this is the first time calling my_pthread_create()
	if (isInitialized == 0){
		isInitialized = 1;	//change isInitialized flag
		my_pthread_t * mainThread = (my_pthread_t *) malloc(sizeof(my_pthread_t)); //malloc space for the my_pthread struct
		mainThread->context = (ucontext_t *) malloc(sizeof(ucontext_t));	//malloc space for main contex
		mainThread->thread_id = 0;	//Zero will always be thread id for main
		getcontext(mainThread->context);	//Saves the current context of main
		mainThread->state = ACTIVE;	//Sets thread to active stat
		tail = enqueue(mainThread, tail, 0);	//Adds main the the priority queue

	}

	//my_pthread_yield();

	// something like this should go here to add the main function to the top of the queue:
	//if queuesize == 0:
	// getcontext(main)
	// add this context to a thread instance
	// increment totalThreads for thread_id
	//add to queue



	/*
	----------NAORIN'S CODE --------------------
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
	*/

}


void my_pthread_yield(){

	/*
		This contains schedule code
	*/

	// check if current thread is done doing stuff
	// if not, add it to a lower priority queue

	int priority = 0;
	
	queue_node* current_thread_node = dequeue(&queue_priority_1); // of ready queue
	//active thread is in priority 1
	if(current_thread_node){
		my_pthread_t *current_thread = current_thread_node->thread;

		if (current_thread->state == ACTIVE && current_thread->state != COMPLETED) {
			current_thread->state = WAITING;
			queue_priority_2 = enqueue(current_thread, queue_priority_2, 2); //send to lower priority queue
		}

		//next thread to be scheduled
		//the thread stays at the top of queue_priority_1 and gets scheduled 
		queue_node* next_thread_node = peek(queue_priority_1);
		if(!next_thread_node){
			priority = 2;
			next_thread_node = peek(queue_priority_2); // if there is no threads left in priority 1, then take it from priority 2, we know there exists one because we enqueued one earlier
		}
		priority = 1;
		my_pthread_t* next_thread = next_thread_node->thread; 
		next_thread->state = ACTIVE;

		current_thread_id = next_thread->thread_id;
		setcontext(next_thread->context);

	}
	else{
		//There are no threads running in priority 1, so we must check priority 2 and schedule from there as well

		current_thread_node = dequeue(&queue_priority_2);\

		//active thread is in priority 2
		if(current_thread_node){
			my_pthread_t *current_thread = current_thread_node->thread;

			if (current_thread->state == ACTIVE && current_thread->state != COMPLETED) {
				current_thread->state = WAITING;
				queue_priority_2 = enqueue(current_thread, queue_priority_2, 2); //send back to lower priority queue
			}
			queue_node *next_thread_node = peek(queue_priority_2); // Run another thread from priority 2. It will re-run the thread that was just taken out of schedule if it is the only one
			priority = 2;
			my_pthread_t *next_thread = next_thread_node->thread;
			next_thread->state = ACTIVE;

			current_thread_id = next_thread->thread_id;
			setcontext(next_thread->context);
		}
		else{
			//there are no threads available
			return;
		}
	}

	// setitimer stuff

	timer.it_interval.tv_usec = 50000*priority;
	timer.it_value.tv_usec = 50000*priority;
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
/*
	// get completed thread and set return value and state
	my_pthread_t* current_thread = dequeueFront();
	current_thread->return_value = value_ptr;
	current_thread->state = COMPLETED;
	// put it back on ready queue so yield can take it off
	enqueueFront(current_thread);
	my_pthread_yield();
*/
	return;


}


int my_pthread_join(my_pthread_t thread, void ** value_ptr){
	

	/*
	1. Calling thread will not run until called thread is done running
	2. Call yield()
	
	*/
/*
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
*/
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

/* PRIORITY QUEUE METHODS */

/*
	enqueue takes a thread, queue_node, and priority as input and creates a new queue_node and places it in the rear of the queue
	Returns the new tail of the queue
	Must be called in the form: queue = enqueue(thread, queue, priority)
*/
queue_node* enqueue(my_pthread_t * newThread, queue_node *tail, int priority){
	queue_node *new_node = malloc(sizeof(queue_node));
	new_node->thread = newThread;
	new_node->priority = priority;
	if(tail==NULL){
		new_node->next = NULL;
		return new_node;
	}
	new_node->next = tail;
	return new_node;
}
/*
	deqeue takes a queue_node as input and removes the last queue_node in the queue
	Returns the last queue_node in the queue
	Must be called in the form: dequeue(&queue)
*/
queue_node* dequeue(queue_node ** tail){
	
	if((*tail) == NULL)
		return NULL;
	queue_node *iter = NULL;
	queue_node *prev = NULL;
	iter = (*tail);
	while(iter->next){
		prev = iter;
		iter = iter->next;
	}
	if(prev){
		prev->next = NULL;
	}else{
		(*tail) = NULL;
	}
	return iter;
}
queue_node* peek(queue_node * tail){
	if(tail == NULL){
		return NULL;
	}
	queue_node *iter = NULL;
	iter = tail;
	while(iter->next){
		iter = iter->next;
	}
	return iter;
}

void printQueue(queue_node *tail){
	queue_node *iter = tail;
	//printf("%s\n", iter->thread->string);
	while(iter){
		printf("%s\n", iter->thread->string);
		iter = iter->next;
	}
}


/*
	This function inputs a new thread at the beginning of the queue
	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure
*/
/*
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
*/

/*
	This function inputs a new thread at the rear of the queue
	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure
*/
/*
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
*/

/*
	This function dequeues an item from the front
	INPUT: The thread to add
	OUTPUT: 1 on succcess, zero on failure
*/
/*
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
*/





void * printFunction(void *arg){
	
	
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
	my_pthread_t * thread = malloc(sizeof(my_pthread_t));
	thread->string = "this is the first thread";
	my_pthread_t * thread2 = malloc(sizeof(my_pthread_t));
	thread2->string = "this is the second thread";
	my_pthread_t * thread3 = malloc(sizeof(my_pthread_t));
	thread3->string = "this is the third thread";
	queue_node * queue = NULL;
	queue = enqueue(thread, queue, 0);
	queue = enqueue(thread2, queue, 0);
	queue = enqueue(thread3, queue, 0);
	printQueue(queue);
	printf("removing: %s\n", dequeue(&queue)->thread->string);
	printf("removing: %s\n", dequeue(&queue)->thread->string);
	printf("removing: %s\n", dequeue(&queue)->thread->string);
*/

	//if(temp->thread->string){
	//	printf("%s\n", temp->thread->string);
	//}


	//void * (*functionPointer)(void *);
	//functionPointer = &printFunction;

	//void(*otherFunction)();
	//otherFunction = &printFunction;

	my_pthread_t * thread;

	my_pthread_create(thread, NULL, &printFunction, NULL);	

	printf("The tail node is %d\n", tail->thread->thread_id);
	printf("The next node is %d\n", tail->next->thread->thread_id);

	//printf("Thread id is %d\n", node->thread_id);

	//ucontext_t * otherContext = node->context;
	//makecontext(otherContext, otherFunction,0);
	setcontext(tail->next->thread->context);	
	

	printf("Ending main\n");
	return 0;
}