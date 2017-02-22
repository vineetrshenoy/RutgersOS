#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 100000
#define INSCHED false

typedef struct thread_node {

	int thread_id;
	char * context;
	struct thread_node * next;

}thread_node;

ucontext_t ucp, ucp_two, ucp_main;
volatile int x;
struct itimerval timer;
thread_node * head;

typedef unsigned long int my_pthread_t;

typedef struct
{
  int __detachstate;
  int __schedpolicy;
  struct sched_param __schedparam;
  int __inheritsched;
  int __scope;
  size_t __guardsize;
  int __stackaddr_set;
  void *__stackaddr;
  unsigned long int __stacksize;
}
my_pthread_attr_t;


//int my_pthread_create(my_pthread_t *thread, pthread_attr_t * attr, void * (*function)(void*), void* arg){

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
		
	ucontext_t a;
	getcontext(&a);
	a.uc_link = 0;
	a.uc_stack.ss_sp = malloc(STACK_SIZE);
	a.uc_stack.ss_size = STACK_SIZE;
	a.uc_stack.ss_flags = 0;
	makecontext(&a, (void*) &function, arg);
	if (!INSCHED) {
		ready_queue.add(getcontext(&a));
	}
	*/




//}


//void my_pthread_yield(){

	/*
		This contains schedule code
	ucontext_t curr;
	getcontext(&curr);
	swapcontext(&curr, ready_queue.pop());
	timer.it_interval.tv_usec = 50000;
	timer.it_value.tv_usec = 50000;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
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

// struct _pthread_fastlock
// {
//   long int __status;
//   int __spinlock;
// }
//  ;

// typedef struct
// {
//   int __m_reserved;
//   int __m_count;
//   _pthread_descr __m_owner;
//   int __m_kind;
//   struct _pthread_fastlock __m_lock;
// }
// my_pthread_mutex_t;

// typedef struct
// {
//   int __mutexkind;
// }
// my_pthread_mutexattr_t;

//int my_thread_mutex_init(my_pthread_t * mutex, const my_pthread_mutexattr_t * mutexattr){

//}


// int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

	

// }

 // int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){

 // }

// int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){

// }












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

	thread_node * first = (thread_node *) malloc (sizeof(thread_node));
	thread_node * second = (thread_node *) malloc (sizeof(thread_node));
	head = first;
	first->context = "This is the first node\n";
	first->next = second;
	second->context = "This is the second node\n";
	second->next = NULL;

	

	printf("%s\n", first->context);
	printf("%s\n", first->next->context);

	


	printf("Ending main\n");
	return 0;
}
