// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: 

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

typedef enum {
	ACTIVE,
	WAITING,
	COMPLETED
} my_pthread_state;

typedef struct my_pthread_t {

	int thread_id;
	ucontext_t * context;
	char * string;
	my_pthread_state state;
	void* return_value;

}my_pthread_t;

typedef struct queue_node {
	int priority;
	my_pthread_t* thread;
	struct queue_node *next;

}queue_node;

typedef struct{} my_pthread_attr_t;

typedef int my_pthread_mutex_t;

typedef struct{} my_pthread_mutexattr_t;

typedef struct my_pthread_mutex_node 
{
	my_pthread_mutex_t *mutex;
	my_pthread_t *holder;
} my_pthread_mutex_node;

int my_pthread_create(my_pthread_t * thread, my_pthread_attr_t * attr, void * (*function) (void*), void* arg);
// Creates a pthread that executes function. Attributes are ignored.

void my_pthread_yield();
// Explicit call to the my_pthread_t scheduler requesting theat the current context be swapped out and another be scheduled.

void my_pthread_exit(void * value_ptr);
// Explicit call to the my_pthread_t library to end the pthread that called it. If the value_ptr isn't NULL, any return value from the thread will be saved.

int my_pthread_join(my_pthread_t thread,  void ** value_ptr);
// Call to the my_pthread_t library ensuring that the calling thread will not execute until the one it references exits. If value_ptr is not null, the return value of the exiting thread will be passed back.

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr);
//Initializes a my_pthread_mutex_t creaded by the calling thread. Attributes are ignored.

int my_pthread_lock(my_pthread_mutex_t *mutex);
// Locks a given mutex, other threads attempting to access this mutex will not run until it is unlocked.

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
// Unlocks a given mutex.

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);
// Desetroys a given mutex. Mutex should be unlocked before doing so.

/*
 enqueue takes a thread, queue_node, and priority as input and creates a new queue_node and places it in the rear of the queue
 Returns the new tail of the queue
 Must be called in the form: queue = enqueue(thread, queue, priority)
*/
queue_node* enqueue(my_pthread_t * newThread, queue_node *tail, int priority);

/*
 deqeue takes a queue_node pointer as input and removes the last queue_node in the queue
 Returns the last queue_node in the queue
 Must be called in the form: dequeue(&queue)
*/
queue_node* dequeue(queue_node ** tail);
/*
 peek takes a queue_node as in put and returns the top node in the queue without modifying it
 Must be called in the form: peek(queue)
*/
queue_node* peek(queue_node * tail);

/* 
 printQueue prints a queue sequentially
*/
void printQueue(queue_node *tail);


#endif
