#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	if (argc < 2) {
		write(STDOUT_FILENO, "Incorrect Usage\nGive a command as input\n", 40);
		_exit(-1);
	}
	/*  The  popen()  function opens a process by creating a pipe, forking,
	 *  and invoking the shell.  Since a pipe is by definition unidirectional, 
	 *  the type argument may specify only reading or writing,not both.
	 */
	FILE *file = popen( argv[1], "r");
	if (file == NULL) {
		perror("popen");
		_exit(-1);
	}
	
	/* Examines a stream and returns its file descriptor*/
	int fd = fileno(file);
	int len = 0;
	char buff[256];

	while ((len = read(fd, buff, 256)) > 0) {
		if (write(STDOUT_FILENO, buff, len) < 0) {
			perror("write");
			_exit(-1);
		}
	}
	pclose(file);
}
