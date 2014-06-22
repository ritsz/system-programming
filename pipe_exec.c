#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define TEST_EXEC_FD 0

int main(void)
{
	int pfds[2];
	if (pipe(pfds) < 0) {
		perror("Pipe");
		_exit(-1);
	}

	int pid = fork();
	if (pid < 0) {
		perror("Fork");
		_exit(-1);
	}
	if (pid == 0) {
		close(pfds[0]);		/*Not required for child*/
		close(STDOUT_FILENO);	/*Close the STDOUT_FILENO*/
		dup(pfds[1]);		/*A copy of pfds[1] is made on the first
					 avaibale file descriptor, essentially
					 STDOUT. Thus anything ls returns to
					 STDOUT, goes to pfds[1] instead, which
					 can be read at pfds[0]*/
		execlp("ls", "ls", "-l", NULL); /* Execute ls command. Upon exec
						 the text part is overlayed by
						 new code. The file descriptors
						 are copied across exec() calls.
						 The data written to pipe
						 pfds[1] is forwarded to
						 pfds[0], which gets written to
						 a file. If we don't do
						 dup(pfds[1]), data will get
						 written to STDOUT, thus showing
						 that fd get copied on exec.*/
	} else {
		close(pfds[1]);		/*Not required for parent*/
		if (!TEST_EXEC_FD) {
			close(STDIN_FILENO);	/*STDIN_FILENO is closed*/
			dup(pfds[0]);		/*pfds[0] copied to STDIN, ie any data
						  avaibale at pfds[0] is assumed to be
						  from STDIN by wc command*/
			execlp("wc", "wc", "-l", NULL);
		}
		int fd = open("LOG", O_RDWR|O_CREAT);
		char buff[2048];
		int ret = read(pfds[0], buff, 2048);
		write(fd, buff, ret);
		close(fd);
	}

	return 0;
}
