#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	int pfds[2];
	if (pipe(pfds) < 0)
		perror("Pipe");

	int pid = fork();
	if (pid < 0)
		perror("Fork");

	if (pid == 0) {
		close(pfds[0]);		/*Not required for child*/
		close(STDOUT_FILENO);	/*Close the STDOUT_FILENO*/
		dup(pfds[1]);		/*A copy of pfds[1] is made on the first
					 avaibale file descriptor, essentially
					 STDOUT. Thus anything ls returns to
					 STDOUT, goes to pfds[1] instead, which
					 can be read at pfds[0]*/
		execlp("ls", "ls", "-l", NULL); /* Execute ls command*/
	} else {
		close(pfds[1]);		/*Not required for parent*/
		close(STDIN_FILENO);	/*STDIN_FILENO is closed*/
		dup(pfds[0]);		/*pfds[0] copied to STDIN, ie any data
					  avaibale at pfds[0] is assumed to be
					  from STDIN by wc command*/
		execlp("wc", "wc", "-l", NULL);
	}

	return 0;
}
