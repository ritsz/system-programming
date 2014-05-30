#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *first(void *args)
{
	write(STDOUT_FILENO, "HELLO WORLD\n", 12);
}

void *second(void *args)
{
	write(STDOUT_FILENO, "GOODBYE WORLD\n", 14);
}


int main(void)
{
	pthread_t thread_id = 1;
	/*
	   pthread_create takes pthread_t tid, pthread_attr_t which is
	   attributes (that specify priority, stacksize and whether thread is
	   daemon), a thread start function. The thread finishes when this
	   function returns or when pthread_exit is called.
	 */
	if (pthread_create(&thread_id, NULL, first, NULL) != 0)
		perror("Pthread create");

	long i = 0;
	
	/*
	   We need to wait for the thread to finish. We can do that by maunually
	   looping/ sleeping or pthread_join which waits for a particular
	   pthread_t

	   while (i < 1000000)
		i++;
	*/
	pthread_join(thread_id, NULL);
	return 0;
}
