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
		dup(pfds[1]);		/*Pipe[1] becomes STDOUT*/
		execlp("ls", "ls", "-l", NULL); /* Execute ls command*/
	} else {
		close(pfds[1]);		/*Not required for parent*/
		close(STDIN_FILENO);	/*STDIN_FILENO is closed*/
		dup(pfds[0]);
		execlp("wc", "wc", "-l", NULL);
	}

	return 0;
}
