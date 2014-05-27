/*
   Program that simulates a client server model using pipes. The client sends a
   path which the server reads and sends data to the client. Tested using the
   device file /dev/eudyptula that was created using miscellaneous character
   device.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int client(int readfd, int writefd, char path[])
{
	int len = strlen(path);
	/* Send path to server*/
	if (write(writefd, path, len) < 0)
		perror("Sending path to server");

	char buff[256];

	/*Read from pipe for data till EOF*/
	while((len = read(readfd, buff, 256)) > 0) {
		write(STDOUT_FILENO, "Client: ", 8);
		write(STDOUT_FILENO, buff, len);
	}

	/*Close all remaining pipes*/
	close(readfd);
	close(writefd);
}

int server(int readfd, int writefd)
{
	/*Read path from client*/
	char buff[256];
	int len = 256;
	if ((len = read(readfd, buff, len)) < 0)
		perror("Path from client");
	
	int filed;
	/*Open file and report to client if file not available*/
	if ((filed = open(buff, O_RDONLY)) < 0) {
		write(writefd, "Error opening file\n", 19);
		_exit(-1);
	}

	/*Write contents of file to pipe till EOF reached*/
	while ((len = read(filed, buff, 256)) > 0)
		write(writefd, buff, len);
	
	/*Close all remaining pipes*/
	close(filed);
	close(readfd);
	close(writefd);
}

int cpfd[2];
int pcfd[2];

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage: ./clspipes <path of file>\n");
		_exit(-1);
	}
		
	if (pipe(pcfd) < 0)
		perror("Pipe");
	if (pipe(cpfd) < 0)
		perror("Pipe");
/* Pipe not made O_NONBLOCL because we want read, writes to block*/
	pid_t cpid = fork();

	if (cpid < 0) {
		perror("Fork");
	} else if (cpid == 0) {
/*Closing pipe ends that are not needed by server */
		close(cpfd[0]);
		close(pcfd[1]);
		server(pcfd[0], cpfd[1]);
		_exit(0);
	} else {
		close(cpfd[1]);
		close(pcfd[0]);
		client(cpfd[0], pcfd[1], argv[1]);
		waitpid(cpid, NULL, 0);
	}
}
