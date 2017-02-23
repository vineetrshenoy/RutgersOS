// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: 

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#include <ucontext.h>

typedef enum {
	ACTIVE,
	WAITING,
	COMPLETED
} my_pthread_state;

typedef unsigned long int my_pthread_t;

typedef struct {
	my_pthread_t thread_id;
	ucontext_t context;
	my_pthread_state state;
	void* return_value;
} my_pthread;

typedef struct {
	int mutex_id;
} my_pthread_mutex_t;

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

#endif
