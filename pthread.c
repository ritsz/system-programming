#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
	
int fid = 0;

/* If locks are not used, read and write changes the file offset while the other
 * process is hapenning, corrupting the file. Having mutexes, prevents such
 * uncontrolled changes of offset
 */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *first(void *args)
{
	pthread_detach(pthread_self());
	char *buff  = "LOREM IPSUM dolor sit amet, consectetur adipiscing elit.\
Suspendisse vestibulum euleo quis semper. Morbi lacinia feugiat\
volutpat. Integer congue tempor risus feugiat laoreet. Etiam \
rhoncus tellus nec risus egestas blandit. Proin non sapien diam.\
Quisque et diam sagittis, mattis leo quis, feugiat urna. Morbi \
viverra vel ligula ac feugiat. Pellentesque iaculis sem nulla, \
tristique iaculis tellus dapibus a. In in vulputate nibh\n\n";
	while (1) {
		pthread_mutex_lock(&mutex);
		lseek(fid, SEEK_SET, 0);	
		if (write(fid, buff, strlen(buff)) < 0) {
			perror("Write thread 1");
			_exit(-1);
		}
		pthread_mutex_unlock(&mutex);
	}
}

void *second(void *args)
{
	pthread_detach(pthread_self());
	char buff2[512];
	int read_chars;
	while (1) {
		lseek(fid, SEEK_SET, 0);
		pthread_mutex_lock(&mutex);
		if ((read_chars = read(fid, buff2, strlen(buff2))) < 0) {
			perror("Read thread 2");
			_exit(-1);
		} else {
			buff2[read_chars] = '\n';
			write(STDOUT_FILENO, buff2, read_chars+1);
		}
		pthread_mutex_unlock(&mutex);
	}
}


int main(void)
{
	/*
	   Creating/ Opening the file that stores common data
	  */
	if ((fid = open("mutex_pthread_file", O_CREAT | O_RDWR)) == 0) {
		perror("Open");
		_exit(-1);
	}

	pthread_t thread_id, second_id;
	/*
	   pthread_create takes pthread_t tid, pthread_attr_t which is
	   attributes (that specify priority, stacksize and whether thread is
	   daemon), a thread start function. The thread finishes when this
	   function returns or when pthread_exit is called.
	 */
	if (pthread_create(&thread_id, NULL, first, NULL) != 0) {
		perror("Pthread create");
		_exit(-1);
	}
	if (pthread_create(&second_id, NULL, second, NULL) != 0) {
		perror("Pthread create");
		_exit(-1);
	}

	/*
	   We need to wait for the thread to finish. We can do that by maunually
	   looping/ sleeping or pthread_join which waits for a particular
	   pthread_t

	   while (i < 1000000)
		i++;
	*/
	pthread_join(second_id, NULL);
	pthread_join(thread_id, NULL);
	return 0;
}
