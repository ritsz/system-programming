#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "test.h"

int pcfd[2];	/*Parent to child pipe*/

int main(void)
{
	write(1, "Making pipes\n", 13);
	if (pipe(pcfd) != 0) {
		perror("Pipe");
		_exit(-1);
	}
	int flags = fcntl(pcfd[0], F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(pcfd[0], F_SETFL, flags);

	flags = fcntl(pcfd[1], F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(pcfd[1], F_SETFL, flags);

	write(1, "Forking a process\n", 18);
	pid_t cpid = fork();

	if (cpid < 0) {
		perror("Fork");
		_exit(-1);
	} else if (cpid == 0) {
		write(1, "Child Process\n", 14);
		char buff[19];
		close(pcfd[1]);
		if (read(pcfd[0], buff, 19) < 0) {
			perror("Child Read");
	/*Setting O_NONBLOCK gives "Resource temporarily unavailable"*/
			_exit(-1);
		} else {
			printf("%s was read from parent\n", buff);
		}
		close(pcfd[0]);
	} else {
		//wait(NULL);
		if (!write(1, "Parent Process\n", 15)) {
			perror("write");
			_exit(-1);
		}
		close(pcfd[0]);/*Close read end of pipe*/
		write(pcfd[1], "System Programming\0", 19);
		close(pcfd[1]);/*Close write end of pipe*/
	}


	return 0;
}
