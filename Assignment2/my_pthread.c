// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: adapter.cs.rutgers.edu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include "mymalloc.h"
#include "my_pthread_t.h"

#define STACK_SIZE 100000
#define MAXTHREADS 128

ucontext_t ucp, ucp_two, ucp_main;

struct itimerval timer;
queue_node * tail;
int threadIDS = 1;
int queueSize = 0;
int isInitialized = 0;
int totalThreads = 0;
int16_t ** pageTables = NULL;
int16_t * masterTable = NULL;
my_pthread_t current = NULL;
queue_node* queue_priority_1 = NULL;
queue_node* queue_priority_2 = NULL;
queue_node * wait_queue = NULL; 
int priority1_size = 0;
int priority2_size = 0;
int wait_size = 0;

void timer_handler (int signum){
	my_pthread_yield();
}



void initializeScheduler(){
	//If this is the first time calling my_pthread_create()
	if (isInitialized == 0){
		isInitialized = 1;	//change isInitialized flag
		int i = 0;
		//creating the page tables
		pageTables = (int16_t **) myallocate(MAXTHREADS * sizeof(int16_t *));
		for (i = 0; i < MAXTHREADS; i++){
			pageTables[i] = (int16_t *) myallocate(5632 * sizeof(int16_t));
		}
		

		my_pthread_t mainThread = myallocate(sizeof(struct my_pthread_t)); //myallocate space for the my_pthread struct
		mainThread->context = (ucontext_t *) myallocate(sizeof(ucontext_t));	//myallocate space for main contex
		mainThread->thread_id = 0;	//Zero will always be thread id for main
		getcontext(mainThread->context);	//Saves the current context of main
		mainThread->state = ACTIVE;	//Sets thread to active stat
		current = mainThread;
		queue_node *main_node = myallocate(sizeof(queue_node));
		main_node->thread = mainThread;
		main_node->priority = 1;
		main_node->join_value = NULL;
		queue_priority_1 = enqueue(main_node, queue_priority_1, &priority1_size);
	}






}

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t * attr, void * (*function)(void*), void* arg){



	// ------VINEET'S CODE ----
	*thread = myallocate(sizeof(struct my_pthread_t)); //myallocate space for new thread
	(*thread)->context = (ucontext_t *) myallocate(sizeof(ucontext_t)); 	//Also myallocate space for ucontext

	//Get context to initialize new thread
	if(getcontext((*thread)->context) == -1){
		printf("Error getting context. Returning -1\n");
		return -1;
	}

	ucontext_t * newContext = (*thread)->context;
	(*newContext).uc_stack.ss_sp = myallocate(STACK_SIZE);	//myallocates new stack space
	(*newContext).uc_stack.ss_size = STACK_SIZE;	//describes size of stack
	(*newContext).uc_link = NULL;
	

	(*thread)->thread_id = threadIDS; //Gives the thread an ID
	threadIDS++;
	(*thread)->state = ACTIVE;	//Sets thread to active stat

	makecontext((*thread)->context, (void (*)()) function, 1, arg); //creates with function. Users usually pass a struct of arguments?
	queue_node *new_node = myallocate(sizeof(queue_node));
	new_node->thread = *thread;
	new_node->priority = 1;
	new_node->join_value = NULL;
	queue_priority_1 = enqueue(new_node, queue_priority_1, &priority1_size);	//Adds thread to priority queue
	
			



	my_pthread_yield();
	return 0;

}


void my_pthread_yield(){

	/*
		This contains schedule code
	*/

	

		// setitimer stuff
	struct sigaction sa;

	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGALRM, &sa, NULL);

	int priority = 0;
	queue_node* current_thread_node = peek(queue_priority_1); // of ready queue
	
	//active thread is in priority 1
	if(current_thread_node){
		my_pthread_t current_thread = current_thread_node->thread;
		printf("1. Thread found in priority 1: %i\n", current_thread->thread_id);



		//next thread to be scheduled
		//the thread stays at the top of queue_priority_1 and gets scheduled 
		

		if(current == NULL){
			/* Start a virtual timer. It counts down whenever this process is
	  		executing. */
	  		current = current_thread;

	  		timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 500000*priority;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 500000*priority;

			setitimer (ITIMER_REAL, &timer, NULL);

			// if(current_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			setcontext(current_thread->context);
		}else if (current->thread_id != current_thread->thread_id){
			my_pthread_t temp = current;
			current = current_thread;

			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 500000*priority;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 500000*priority;

			setitimer(ITIMER_REAL, &timer, NULL);
			// if(current_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			swapcontext(temp->context, current_thread->context);
		}else{
			current_thread_node = dequeue(&queue_priority_1, &priority1_size);
			current_thread = current_thread_node->thread;
			if (current_thread->state == ACTIVE && current_thread->state != COMPLETED) {
				
				printf("moving thread to priority 2: %i\n", current_thread->thread_id);
				current_thread->state = ACTIVE;
				current_thread_node->priority = 2;
				queue_priority_2 = enqueue(current_thread_node, queue_priority_2, &priority2_size); //send to lower priority queue
				
			}
			queue_node* next_thread_node = peek(queue_priority_1);
			priority = 1;
			if(next_thread_node==NULL){
				priority = 2;
				next_thread_node = peek(queue_priority_2); // if there is no threads left in priority 1, then take it from priority 2, we know there exists one because we enqueued one earlier
			}
			my_pthread_t next_thread = next_thread_node->thread; 

			printf("2. Next thread found (to be scheduled), in priority %i: %i\n", priority, next_thread->thread_id);

			next_thread->state = ACTIVE;
			my_pthread_t temp = current;
			current = next_thread;

			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 500000*priority;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 500000*priority;

			setitimer(ITIMER_REAL, &timer, NULL);
			// if(next_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			swapcontext(temp->context, next_thread->context);
		}
	}
	else{
		//There are no threads running in priority 1, so we must check priority 2 and schedule from there as well

		current_thread_node = peek(queue_priority_2);
		

		//active thread is in priority 2
		if(current_thread_node){
			my_pthread_t current_thread = current_thread_node->thread;

			printf("3. Thread found in priority 2: %i\n", current_thread->thread_id);
			


			if(current == NULL){
				/* Start a virtual timer. It counts down whenever this process is
		  		executing. */
		  		current = current_thread;

		  		timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 500000*priority;
				timer.it_interval.tv_sec = 0;
				timer.it_interval.tv_usec = 500000*priority;

				setitimer (ITIMER_REAL, &timer, NULL);
				// if(current_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				setcontext(current_thread->context);
			}else if (current->thread_id != current_thread->thread_id){
				my_pthread_t temp = current;
				current = current_thread;

				timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 500000*priority;
				timer.it_interval.tv_sec = 0;
				timer.it_interval.tv_usec = 500000*priority;

				setitimer(ITIMER_REAL, &timer, NULL);
				// if(current_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				swapcontext(temp->context, current_thread->context);
			}else{
				current_thread_node = dequeue(&queue_priority_2, &priority2_size);
				current_thread = current_thread_node->thread;
				if (current_thread->state == ACTIVE && current_thread->state != COMPLETED) {
					current_thread->state = ACTIVE;
					current_thread_node->priority = 2;
					queue_priority_2 = enqueue(current_thread_node, queue_priority_2, &priority2_size); //send back to lower priority queue
					
					printf("Thread found being sent back to priority 2: %i\n", current_thread->thread_id);
				}
				queue_node *next_thread_node = peek(queue_priority_2); // Run another thread from priority 2. It will re-run the thread that was just taken out of schedule if it is the only one
				priority = 2;
				my_pthread_t next_thread = next_thread_node->thread;
				next_thread->state = ACTIVE;
				printf("4. Next thread found (to be scheduled), in priority 2: %i\n", next_thread->thread_id);
				my_pthread_t temp = current;
				current = next_thread;

				timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 500000*priority;
				timer.it_interval.tv_sec = 0;
				timer.it_interval.tv_usec = 500000*priority;

				setitimer(ITIMER_REAL, &timer, NULL);
				// if(next_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				swapcontext(temp->context, next_thread->context);
			}
		}
		else{
			printf("5. There are no threads available\n");
			//there are no threads available
			return;
		}
	}

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
	my_pthread_t current_thread = current;
	queue_node *queue_priority_1_head = peek(queue_priority_1);
	queue_node *queue_priority_2_head = peek(queue_priority_2);

	int check = 1;

	int removed_node_id = 0;

	if(queue_priority_1_head){
		my_pthread_t queue_priority_1_head_thread = queue_priority_1_head->thread;
		queue_node * removed_node = NULL;
		if(current_thread->thread_id == queue_priority_1_head_thread->thread_id){
			removed_node = dequeue(&queue_priority_1, &priority1_size);
			removed_node_id = removed_node->thread->thread_id;

			removed_node->thread->state = COMPLETED;
			if (value_ptr != NULL) {
				removed_node->thread->return_value = value_ptr;
			}
			// mydeallocate(removed_node->thread);
			mydeallocate(removed_node);
			check = 0;
		}
	}
	if(check==1 && queue_priority_2_head){
		queue_node * removed_node = NULL;
		my_pthread_t queue_priority_2_head_thread = queue_priority_2_head->thread;
		removed_node = dequeue(&queue_priority_2, &priority2_size);
		removed_node_id = removed_node->thread->thread_id;
		
		removed_node->thread->state = COMPLETED;
		if (value_ptr != NULL) {
			removed_node->thread->return_value = value_ptr;
		}
		// mydeallocate(removed_node->thread);
		mydeallocate(removed_node);
	}
	// else if (current->thread_id == 0) {
	// 	current->state = COMPLETED;
	// 	if (value_ptr != NULL) {
	// 		current->return_value = value_ptr;
	// 	}
	// 	printf("exiting thread: %d\n", current->thread_id);
	// 	current = NULL;
	// 	return;
	// }



	
	// queue_node * iter; 
	// int queueSize = wait_size;	//number of nodes in the wait queue
	// int i;	//counter
	// for (i = 0; i < queueSize; i++){
	// 	iter = dequeue(&wait_queue, &wait_size);	//dequeue a iter
	// 	//if the current thread is in the waitqueue
	// 	if (iter->thread->thread_id == removed_node_id){
	// 		if(iter->join_value != NULL) {
	// 			void **join_value_temp = iter->join_value;
	// 			*join_value_temp = value_ptr;
	// 		}
	// 		queue_priority_1 = enqueue(iter, queue_priority_1, &priority1_size);
			
	// 	}
		
	// 	else{
	// 	wait_queue = enqueue(iter, wait_queue, &wait_size);
	// 	}
	// }
	
	printf("exiting thread: %d\n", current->thread_id);
	current = NULL;
	my_pthread_yield();
	
	return;
}


int my_pthread_join(my_pthread_t thread, void ** value_ptr){
	
	//Check if it is in the wait_queue. if not, return error
	// if (search_wq() == 1){
	// 	printf("Unable to join\n");
	// 	return -1;
	// }
	//get the node from the priority queue
	printf("join was called.\n");
	// queue_node * node = search_pq();
	// if (node == NULL){
	// 	printf("Unable to join\n");
	// 	return -1;
	// }

	while (thread->state != COMPLETED) {
		printf("waiting on %d\n", thread->thread_id);
		my_pthread_yield();
	}

	//set the id on which this thread is waiting and add to queue
	// node->waiting_id = thread.thread_id;

	if (value_ptr != NULL) {
		// node->join_value = value_ptr;
		*value_ptr = thread->return_value;
	}

	mydeallocate(thread->context->uc_stack.ss_sp);
	mydeallocate(thread->context);
	mydeallocate(thread);


	// wait_queue = enqueue(node, wait_queue, &wait_size);

	// my_pthread_yield();

	return 0;

}




// int FetchAndAdd(int *ptr) {
// 	int old = *ptr;
// 	*ptr = old + 1;
// 	return old;
// }

int my_pthread_mutex_init(my_pthread_mutex_t * mutex, const my_pthread_mutexattr_t * mutexattr){

	// mutex queue is needed
	// new mutex node added to mutex queue
	// return 0 for success, -1 for failure

	mutex->ticket = 0;
	mutex->turn = 0;
	return 0;

}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

	


	int myturn = __sync_fetch_and_add(&mutex->ticket, 1);
	while (mutex->turn != myturn)
		my_pthread_yield(); //spin
	return 0;
	
}

 int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){

	

 	mutex->turn = mutex->turn + 1;

 	return 0;

 }

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){

	
	
	mutex = NULL;
	return 0;

}

queue_node * search_pq(){
	int i;
	queue_node * node;
	if (queue_priority_1 != NULL){
		int queueSizeOne = priority1_size;
		
		
		for (i = 0; i < queueSizeOne; i++){
			node = dequeue(&queue_priority_1, &priority1_size);
			if (node->thread->thread_id == current->thread_id)
				return node;
			queue_priority_1 = enqueue(node, queue_priority_1, &priority1_size);
		}
	}
	else if (queue_priority_2 != NULL){
		int queueSizeTwo = priority2_size;
		for (i = 0; i < queueSizeTwo; i++){
			node = dequeue(&queue_priority_2, &priority2_size);
			if (node->thread->thread_id == current->thread_id)
				return node;
			queue_priority_2 = enqueue(node, queue_priority_2, &priority2_size);
		}
			
	}


	return NULL;

}



int search_wq(){
	//if the waitqueue has no objects, return null;
	if(wait_queue == NULL)
		return 0;
	queue_node * node; 
	int queueSize = wait_size;	//number of nodes in the wait queue
	int i;	//counter
	for (i = 0; i < queueSize; i++){
		node = dequeue(&wait_queue, &wait_size);	//dequeue a node
		//if the current thread is in the waitqueue
		if (node->thread->thread_id == current->thread_id){
			wait_queue = enqueue(node, wait_queue, &wait_size);
			return 1;
		}
		wait_queue = enqueue(node, wait_queue, &wait_size);
	}
	return 0;

}

/* PRIORITY QUEUE METHODS */

/*
	enqueue takes a thread, queue_node, and priority as input and creates a new queue_node and places it in the rear of the queue
	Returns the new tail of the queue
	Must be called in the form: queue = enqueue(thread, queue, priority)
*/
queue_node* enqueue(queue_node * new_node, queue_node *tail,int * queue_size){
	
	if(tail==NULL){
		new_node->next = NULL;
		(*queue_size)++;
		return new_node;
	}
	new_node->next = tail;
	(*queue_size)++;
	return new_node;
}
/*
	deqeue takes a queue_node as input and removes the last queue_node in the queue
	Returns the last queue_node in the queue
	Must be called in the form: dequeue(&queue)
*/
queue_node* dequeue(queue_node ** tail, int * queue_size){
	
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
	(*queue_size)--;
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



void * printFunction(void *arg){
	printf("Waiting for signal handler\n");
	sleep(4);
	printf("We are here now \n");
	my_pthread_exit(NULL);
}
void * counterFunction(void *arg){
	int i;
	for (i = 0; i < 20; i++)
	{
		printf("%i\n", i);
		sleep(1);
	}
	my_pthread_exit(NULL);
}


void handler(int sig){
	printf("In signal handler\n");
	setcontext(&ucp_main);
}


/*
int main(){

	
	my_pthread_t *thread;
	my_pthread_t *thread2;
	my_pthread_create(thread, NULL, &counterFunction, NULL);
	my_pthread_create(thread2, NULL, &printFunction, NULL);
	my_pthread_join(*thread2, NULL);
	my_pthread_yield();

	
	my_pthread_t * thread = myallocate(sizeof(my_pthread_t));
	thread->string = "this is the first thread";
	my_pthread_t * thread2 = myallocate(sizeof(my_pthread_t));
	thread2->string = "this is the second thread";
	my_pthread_t * thread3 = myallocate(sizeof(my_pthread_t));
	thread3->string = "this is the third thread";
	queue_node * queue = NULL;
	queue = enqueue(thread, queue, 0);
	queue = enqueue(thread2, queue, 0);
	queue = enqueue(thread3, queue, 0);
	printQueue(queue);
	printf("removing: %s\n", dequeue(&queue)->thread->string);
	printf("removing: %s\n", dequeue(&queue)->thread->string);
	printf("removing: %s\n", dequeue(&queue)->thread->string);




	printf("Ending main\n");
	
	return 0;

}
*/