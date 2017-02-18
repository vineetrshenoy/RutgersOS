int my_pthread_create(pthread_t * thread, pthread_attr_t * attr, void * (*function) (void*), void* arg);
void my_pthread_yield();
void pthread_exit(void * value_ptr);
int my_pthread_join(pthread_t thread,  void ** value_ptr);