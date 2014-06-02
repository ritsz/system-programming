#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	if (argc < 2) {
		write(STDOUT_FILENO, "Incorrect Usage\n", 16);
		_exit(-1);
	}
	FILE *file = popen( argv[1], "r");
	if (file == NULL) {
		perror("popen");
		_exit(-1);
	}

	int fd = fileno(file);
	int len = 0;
	char buff[256];

	while ((len = read(fd, buff, 256)) > 0) {
		write(STDOUT_FILENO, buff, len);
	}
	pclose(file);
}
