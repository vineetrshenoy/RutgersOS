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





ucontext_t ucp, ucp_two, ucp_main;

struct itimerval timer;
queue_node * tail;
int threadIDS = 1;
int queueSize = 0;
int isInitialized = 0;
int mainEnqueued = 0;
int totalThreads = 0;
int16_t ** pageTables = NULL;
char * masterTable = NULL;
my_pthread_t current = NULL;
my_pthread_t pointer = NULL;
queue_node* queue_priority_1 = NULL;
queue_node* queue_priority_2 = NULL;
queue_node * wait_queue = NULL; 
int priority1_size = 0;
int priority2_size = 0;
int wait_size = 0;
extern int fileDescriptor;
extern int OS_SIZE;
extern int pageSize;
extern void * memory;

void timer_handler (int signum){
	my_pthread_yield();
}



void initializeScheduler(){
	void *ptr, *initptr;
	int i;
	//If this is the first time calling my_pthread_create()
	if (isInitialized == 0){
		isInitialized = 1;	//change isInitialized flag
		int i = 0;
		//creating the page tables
		pageTables = (int16_t **) myallocate(MAXTHREADS * sizeof(int16_t *),__FILE__,__LINE__, 69);
		/*for (i = 0; i < MAXTHREADS; i++){
			pageTables[i] = (int16_t *) myallocate(TOTALPAGES * sizeof(int16_t),__FILE__,__LINE__, 69);
		}*/
		masterTable = (char *) myallocate(TOTALPAGES*sizeof(char), __FILE__, __LINE__, 69);
		for (i = 0; i < TOTALPAGES; i++) {
			masterTable[i] = '0';
		}
		for (i = 0; i < MEMORYPAGES; i++) {
			initptr = memory + OS_SIZE + i*pageSize;
			mprotect(initptr, pageSize, PROT_NONE);
		}
		pageTables[0] = (int16_t *) myallocate(MEMORYPAGES * sizeof(int16_t),__FILE__,__LINE__, 69);
		my_pthread_t mainThread = myallocate(sizeof(struct my_pthread_t),__FILE__,__LINE__, 69); //myallocate space for the my_pthread struct
		mainThread->context = (ucontext_t *) myallocate(sizeof(ucontext_t),__FILE__,__LINE__, 69);	//myallocate space for main contex
		mainThread->thread_id = 0;	//Zero will always be thread id for main
		getcontext(mainThread->context);	//Saves the current context of main
		mainThread->state = ACTIVE;	//Sets thread to active stat
		current = mainThread;
		
		pageTables[0][0] = (int16_t) 0;
		pageTables[0][MEMORYPAGES - 1] = (int16_t) MEMORYPAGES - 1;
		for (i = 1; i < MEMORYPAGES - 1; i++) {
			pageTables[0][i] = (int16_t) 6969;
		}
		/*
		for (i = MEMORYPAGES; i < TOTALPAGES; i++) {
			pageTables[0][i] = (int16_t) 6969;
		}
		*/
		masterTable[0] = '1';
		masterTable[MEMORYPAGES - 1] = '1';
		ptr = memory + OS_SIZE;
		mprotect(ptr, pageSize, PROT_READ|PROT_WRITE);
		ptr = memory + OS_SIZE + (MEMORYPAGES - 1)*pageSize;
		mprotect(ptr, pageSize, PROT_READ|PROT_WRITE);
	} 






}

int16_t nextFreePage() {
	int16_t i;
	for (i = 0; i < TOTALPAGES; i++) {
		if (masterTable[i] == '0') {
			printf("NEXT FREE PAGE: %d\n", i);
			return i;
		}
	}
}

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t * attr, void * (*function)(void*), void* arg){

	int16_t pageIndex, endIndex, returnIndexHeader, returnIndexFooter, newAddr;
	void *headerPtr, *footerPtr, *newArg, *ptr;
	int i, sizeOfArg, storedi;
	my_pthread_t savedCurrent;

	newArg = NULL;
	savedCurrent = NULL;

	// ------VINEET'S CODE ----
	*thread = myallocate(sizeof(struct my_pthread_t),__FILE__,__LINE__, 69); //myallocate space for new thread
	(*thread)->context = (ucontext_t *) myallocate(sizeof(ucontext_t),__FILE__,__LINE__, 69); 	//Also myallocate space for ucontext

	//Get context to initialize new thread
	if(getcontext((*thread)->context) == -1){
		printf("Error getting context. Returning -1\n");
		return -1;
	}

	ucontext_t * newContext = (*thread)->context;
	(*newContext).uc_stack.ss_sp = myallocate(STACK_SIZE,__FILE__,__LINE__, 69);	//myallocates new stack space
	(*newContext).uc_stack.ss_size = STACK_SIZE;	//describes size of stack
	(*newContext).uc_link = NULL;
	

	(*thread)->thread_id = threadIDS; //Gives the thread an ID
	threadIDS++;
	(*thread)->state = ACTIVE;	//Sets thread to active stat

	
	/*
	if (arg != NULL) {
		sizeOfArg = sizeof(*arg);
		savedCurrent = current;
		current = *thread;
		returnIndexHeader = swap_out(0);
		swap_in(pageIndex, 0);
		returnIndexFooter = swap_out(MEMORYPAGES - 1);
		swap_in(endIndex, MEMORYPAGES - 1);
		newArg = (void *) myallocate(sizeOfArg, __FILE__, __LINE__, 999);
		*((char *) newArg) = *((char *) arg);
	}
	*/
	makecontext((*thread)->context, (void (*)()) function, 1, arg); //creates with function. Users usually pass a struct of arguments?
	queue_node *new_node = myallocate(sizeof(queue_node),__FILE__,__LINE__, 69);
	new_node->thread = *thread;
	new_node->priority = 1;
	new_node->join_value = NULL;
	queue_priority_1 = enqueue(new_node, queue_priority_1, &priority1_size);	//Adds thread to priority queue
	if (mainEnqueued == 0){
		mainEnqueued = 1;
		queue_node *main_node = myallocate(sizeof(queue_node),__FILE__,__LINE__, 69);
		main_node->thread = current;
		main_node->priority = 1;
		main_node->join_value = NULL;
		queue_priority_1 = enqueue(main_node, queue_priority_1, &priority1_size);
	}

	pageTables[(*thread)->thread_id] = (int16_t *) myallocate(MEMORYPAGES * sizeof(int16_t),__FILE__,__LINE__, 69);
	pageIndex = nextFreePage();
	pageTables[(*thread)->thread_id][0] = pageIndex;
	masterTable[pageIndex] = '1';
	headerPtr = memory + OS_SIZE + pageIndex*pageSize;
	mprotect(headerPtr, pageSize, PROT_READ|PROT_WRITE);
	createPageHeader(pageIndex);
	endIndex = nextFreePage();
	pageTables[(*thread)->thread_id][MEMORYPAGES - 1] = endIndex;
	masterTable[endIndex] = '1';
	footerPtr = memory + OS_SIZE + (endIndex)*pageSize;
	mprotect(footerPtr, pageSize, PROT_READ|PROT_WRITE);
	createPageFooter(endIndex);
	for (i = 1; i < MEMORYPAGES - 1; i++) {
		pageTables[(*thread)->thread_id][i] = (int16_t) 6969;
	}

	
	/*
	for (i = MEMORYPAGES; i < TOTALPAGES; i++) {
		pageTables[(*thread)->thread_id][i] = (int16_t) 6969;
	}
	*/
	/*
	i = 0;
	while (pageTables[current->thread_id][i] != 6969) {
		if (pageTables[current->thread_id][i] == i) {
			newAddr = swap_out(i);
			pageTables[current->thread_id][i] = newAddr;
			if (newAddr < MEMORYPAGES) {
				ptr = memory + OS_SIZE + newAddr*pageSize;
				mprotect(ptr, pageSize, PROT_NONE);
			}
			masterTable[newAddr] = '1';
		}
		i++;
	}
	storedi = i;
	if (pageTables[current->thread_id][MEMORYPAGES - 1] == MEMORYPAGES - 1) {
		newAddr = swap_out(MEMORYPAGES - 1);
		pageTables[current->thread_id][MEMORYPAGES - 1] = newAddr;
		if (newAddr < MEMORYPAGES) {
			ptr = memory + OS_SIZE + newAddr*pageSize;
			mprotect(ptr, pageSize, PROT_NONE);
		}
		masterTable[newAddr] = '1';
	}
	for (i = 0; i < storedi; i++) {
		masterTable[i] = '0';
	}
	masterTable[MEMORYPAGES - 1] = '0';
	if (savedCurrent != NULL) {
		current = savedCurrent;
	}
	i = 0;
	while (pageTables[current->thread_id][i] != 6969) {
		if (pageTables[current->thread_id][i] != i) {
			swap_in(pageTables[current->thread_id][i], i);
			if (i < MEMORYPAGES) {
				ptr = memory + OS_SIZE + i*pageSize;
				mprotect(ptr, pageSize, PROT_READ|PROT_WRITE);
			}
			masterTable[i] = '1';
		}
		i++;
	}
	storedi = i;
	if (pageTables[current->thread_id][MEMORYPAGES - 1] != MEMORYPAGES - 1) {
		swap_in(pageTables[current->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
		ptr = memory + OS_SIZE + (MEMORYPAGES - 1)*pageSize;
		mprotect(ptr, pageSize, PROT_READ|PROT_WRITE);
		masterTable[MEMORYPAGES - 1] = '1';
	}
	for (i = 0; i < storedi; i++) {
		masterTable[pageTables[current->thread_id][i]] = '0';
		pageTables[current->thread_id][i] = (int16_t) i;
	}
	masterTable[pageTables[current->thread_id][MEMORYPAGES - 1]] = '0';
	pageTables[current->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
	*/

	headerPtr = memory + OS_SIZE + pageIndex*pageSize;
	mprotect(headerPtr, pageSize, PROT_NONE);
	footerPtr = memory + OS_SIZE + endIndex*pageSize;
	mprotect(footerPtr, pageSize, PROT_NONE);
	
	my_pthread_yield();
	return 0;

}


void my_pthread_yield(){

	/*
		This contains schedule code
	*/
	

		// setitimer stuff
	struct sigaction sa;
	int i, j, k, storedi;
	int16_t newAddr;

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
		
		// exit was called before this
		if(current == NULL){
			/* Start a virtual timer. It counts down whenever this process is
	  		executing. */
	  		current = current_thread;

	  		timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 500000*priority;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 500000*priority;

			setitimer (ITIMER_VIRTUAL, &timer, NULL);

			// if(current_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			i = 0;
			while(pageTables[current_thread->thread_id][i] != (int16_t) 6969) {
				if (pageTables[current_thread->thread_id][i] != (int16_t) i) {
					j = 0;
					while(pageTables[j]) {
						for (k = 0; k < MEMORYPAGES; k++) {
							if (pageTables[j][k] == (int16_t) i) {
								newAddr = swap_out((int16_t) i);
								// masterTable[i] = '0';
								pageTables[j][k] = newAddr;
								masterTable[newAddr] = '1';
								break;
							}
						}
						j++;
					}
					swap_in(pageTables[current_thread->thread_id][i], i);
					masterTable[pageTables[current_thread->thread_id][i]] = '0';
				}
				pageTables[current_thread->thread_id][i] = (int16_t) i;
				masterTable[i] = '1';
				i++;
			}
			if (pageTables[current_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) { 
				j = 0;
				while (pageTables[j]) {
					if(pageTables[j][MEMORYPAGES - 1] == (int16_t) (MEMORYPAGES - 1)) {
						newAddr = swap_out((int16_t) (MEMORYPAGES - 1));
						// masterTable[MEMORYPAGES - 1] = '0';
						pageTables[j][MEMORYPAGES - 1] = newAddr;
						masterTable[newAddr] = '1';
						break;
					}
					j++;
				}
				swap_in(pageTables[current_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
				masterTable[pageTables[current_thread->thread_id][MEMORYPAGES - 1]] = '0';
			}
			pageTables[current_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
			masterTable[MEMORYPAGES - 1] = '1';
			setcontext(current_thread->context);
		}else if (current->thread_id != current_thread->thread_id){
			// current is main thread
			my_pthread_t temp = current;
			current = current_thread;

			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 500000*priority;
			timer.it_interval.tv_sec = 0;
			timer.it_interval.tv_usec = 500000*priority;

			setitimer(ITIMER_VIRTUAL, &timer, NULL);
			// if(current_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			i = 0;
			while(pageTables[temp->thread_id][i] != (int16_t) 6969) {
				newAddr = swap_out((int16_t) i);
				// masterTable[i] = '0';
				pageTables[temp->thread_id][i] = (int16_t) newAddr;
				masterTable[newAddr] = '1';
				i++;
			}
			storedi = i;
			newAddr = swap_out((int16_t) MEMORYPAGES - 1);
			masterTable[MEMORYPAGES - 1] = '0';
			pageTables[temp->thread_id][MEMORYPAGES - 1] = (int16_t) newAddr;
			masterTable[newAddr] = '1';
			for (i = 0; i < storedi; i++) {
				masterTable[i] = '0';
			}
			i = 0;
			while(pageTables[current_thread->thread_id][i] != (int16_t) 6969) {
				if (pageTables[current_thread->thread_id][i] != (int16_t) i) {
					swap_in(pageTables[current_thread->thread_id][i], i);
					masterTable[pageTables[current_thread->thread_id][i]] = '0';
				}
				pageTables[current_thread->thread_id][i] = (int16_t) i;
				masterTable[i] = '1';
				i++;
			}
			if (pageTables[current_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) { 
				swap_in(pageTables[current_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
				masterTable[pageTables[current_thread->thread_id][MEMORYPAGES - 1]] = '0';
			}
			pageTables[current_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
			masterTable[MEMORYPAGES - 1] = '1';
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

			setitimer(ITIMER_VIRTUAL, &timer, NULL);
			// if(next_thread->thread_id == 0) {
			// 	isInitialized = 0;
			// }
			i = 0;
			while(pageTables[temp->thread_id][i] != (int16_t) 6969) {
				newAddr = swap_out((int16_t) i);
				// masterTable[i] = '0';
				pageTables[temp->thread_id][i] = (int16_t) newAddr;
				masterTable[newAddr] = '1';
				i++;
			}
			storedi = i;
			newAddr = swap_out((int16_t) MEMORYPAGES - 1);
			masterTable[MEMORYPAGES - 1] = '0';
			pageTables[temp->thread_id][MEMORYPAGES - 1] = (int16_t) newAddr;
			masterTable[newAddr] = '1';
			for (i = 0; i < storedi; i++) {
				masterTable[i] = '0';
			}
			i = 0;
			while(pageTables[next_thread->thread_id][i] != (int16_t) 6969) {
				if (pageTables[next_thread->thread_id][i] != (int16_t) i) {
					swap_in(pageTables[next_thread->thread_id][i], i);
					masterTable[pageTables[next_thread->thread_id][i]] = '0';
				}
				pageTables[next_thread->thread_id][i] = (int16_t) i;
				masterTable[i] = '1';
				i++;
			}
			if (pageTables[next_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) {
				swap_in(pageTables[next_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
				masterTable[pageTables[next_thread->thread_id][MEMORYPAGES - 1]] = '0';
			}
			pageTables[next_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
			masterTable[MEMORYPAGES - 1] = '1';
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
			

			// exit was called before this
			if(current == NULL){
				/* Start a virtual timer. It counts down whenever this process is
		  		executing. */
		  		current = current_thread;

		  		timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 500000*priority;
				timer.it_interval.tv_sec = 0;
				timer.it_interval.tv_usec = 500000*priority;

				setitimer (ITIMER_VIRTUAL, &timer, NULL);
				// if(current_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				i = 0;
				while(pageTables[current_thread->thread_id][i] != (int16_t) 6969) {
					if (pageTables[current_thread->thread_id][i] != (int16_t) i) {
						j = 0;
						while(pageTables[j]) {
							for (k = 0; k < MEMORYPAGES; k++) {
								if (pageTables[j][k] == (int16_t) i) {
									newAddr = swap_out((int16_t) i);
									// masterTable[i] = '0';
									pageTables[j][k] = newAddr;
									masterTable[newAddr] = '1';
									break;
								}
							}
							j++;
						}
						swap_in(pageTables[current_thread->thread_id][i], i);
						masterTable[pageTables[current_thread->thread_id][i]] = '0';
					}
					pageTables[current_thread->thread_id][i] = (int16_t) i;
					masterTable[i] = '1';
					i++;
				}
				if (pageTables[current_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) { 
					j = 0;
					while (pageTables[j]) {
						if(pageTables[j][MEMORYPAGES - 1] == (int16_t) (MEMORYPAGES - 1)) {
							newAddr = swap_out((int16_t) (MEMORYPAGES - 1));
							// masterTable[MEMORYPAGES - 1] = '0';
							pageTables[j][MEMORYPAGES - 1] = newAddr;
							masterTable[newAddr] = '1';
							break;
						}
						j++;
					}
					swap_in(pageTables[current_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
					masterTable[pageTables[current_thread->thread_id][MEMORYPAGES - 1]] = '0';
				}
				pageTables[current_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
				masterTable[MEMORYPAGES - 1] = '1';
				setcontext(current_thread->context);
			}else if (current->thread_id != current_thread->thread_id){
				// current is main thread
				my_pthread_t temp = current;
				current = current_thread;

				timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 500000*priority;
				timer.it_interval.tv_sec = 0;
				timer.it_interval.tv_usec = 500000*priority;

				setitimer(ITIMER_VIRTUAL, &timer, NULL);
				// if(current_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				i = 0;
				while(pageTables[temp->thread_id][i] != (int16_t) 6969) {
					newAddr = swap_out((int16_t) i);
					// masterTable[i] = '0';
					pageTables[temp->thread_id][i] = (int16_t) newAddr;
					masterTable[newAddr] = '1';
					i++;
				}
				storedi = i;
				newAddr = swap_out((int16_t) MEMORYPAGES - 1);
				masterTable[MEMORYPAGES - 1] = '0';
				pageTables[temp->thread_id][MEMORYPAGES - 1] = (int16_t) newAddr;
				masterTable[newAddr] = '1';
				for (i = 0; i < storedi; i++) {
					masterTable[i] = '0';
				}
				i = 0;
				while(pageTables[current_thread->thread_id][i] != (int16_t) 6969) {
					if (pageTables[current_thread->thread_id][i] != (int16_t) i) {
						swap_in(pageTables[current_thread->thread_id][i], i);
						masterTable[pageTables[current_thread->thread_id][i]] = '0';
					}
					pageTables[current_thread->thread_id][i] = (int16_t) i;
					masterTable[i] = '1';
					i++;
				}
				if (pageTables[current_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) { 
					swap_in(pageTables[current_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
					masterTable[pageTables[current_thread->thread_id][MEMORYPAGES - 1]] = '0';
				}
				pageTables[current_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
				masterTable[MEMORYPAGES - 1] = '1';
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

				setitimer(ITIMER_VIRTUAL, &timer, NULL);
				// if(next_thread->thread_id == 0) {
				// 	isInitialized = 0;
				// }
				i = 0;
				while(pageTables[temp->thread_id][i] != (int16_t) 6969) {
					newAddr = swap_out((int16_t) i);
					// masterTable[i] = '0';
					pageTables[temp->thread_id][i] = (int16_t) newAddr;
					masterTable[newAddr] = '1';
					i++;
				}
				storedi = i;
				newAddr = swap_out((int16_t) MEMORYPAGES - 1);
				masterTable[MEMORYPAGES - 1] = '0';
				pageTables[temp->thread_id][MEMORYPAGES - 1] = (int16_t) newAddr;
				masterTable[newAddr] = '1';
				for (i = 0; i < storedi; i++) {
					masterTable[i] = '0';
				}
				i = 0;
				while(pageTables[next_thread->thread_id][i] != (int16_t) 6969) {
					if (pageTables[next_thread->thread_id][i] != (int16_t) i) {
						swap_in(pageTables[next_thread->thread_id][i], i);
						masterTable[pageTables[next_thread->thread_id][i]] = '0';
					}
					pageTables[next_thread->thread_id][i] = (int16_t) i;
					masterTable[i] = '1';
					i++;
				}
				if (pageTables[next_thread->thread_id][MEMORYPAGES - 1] != (int16_t) (MEMORYPAGES - 1)) {
					swap_in(pageTables[next_thread->thread_id][MEMORYPAGES - 1], MEMORYPAGES - 1);
					masterTable[pageTables[next_thread->thread_id][MEMORYPAGES - 1]] = '0';
				}
				pageTables[next_thread->thread_id][MEMORYPAGES - 1] = (int16_t) (MEMORYPAGES - 1);
				masterTable[MEMORYPAGES - 1] = '1';
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
	int i;
	my_pthread_t temp;
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
			mydeallocate(removed_node,__FILE__,__LINE__, 69);
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
		mydeallocate(removed_node,__FILE__,__LINE__, 69);
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
	
	
	if (current->thread_id == 0) {
		close(fileDescriptor);
	}
	printf("exiting thread: %d\n", current->thread_id);
	//temp = current;
	pointer = current;
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
	int i;
	void *pagePtr;
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

	i = 0;
	while (pageTables[thread->thread_id][i] != (int16_t) 6969) {
		if (pageTables[thread->thread_id][i] < MEMORYPAGES) {
			pagePtr = memory + OS_SIZE + pageTables[thread->thread_id][i]*pageSize;
			mprotect(pagePtr, pageSize, PROT_READ|PROT_WRITE);
		}
		masterTable[pageTables[thread->thread_id][i]] = '0';
		i++;
	}
	if (pageTables[thread->thread_id][MEMORYPAGES - 1] < MEMORYPAGES) {
		pagePtr = memory + OS_SIZE + pageTables[thread->thread_id][MEMORYPAGES - 1]*pageSize;
		mprotect(pagePtr, pageSize, PROT_READ|PROT_WRITE);
	}
	masterTable[pageTables[thread->thread_id][MEMORYPAGES - 1]] = '0';

	mydeallocate(pageTables[thread->thread_id], __FILE__, __LINE__, 69);
	mydeallocate(thread->context->uc_stack.ss_sp,__FILE__,__LINE__, 69);
	mydeallocate(thread->context,__FILE__,__LINE__, 69);
	mydeallocate(thread,__FILE__,__LINE__, 69);


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
		//printf("%s\n", iter->thread->string);
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
