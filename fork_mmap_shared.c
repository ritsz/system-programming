#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int fd, pid, size_file;
	char *buff;
	struct stat stat_buf;

	if ((fd = open("testfile.txt", O_RDWR)) < 0) {
		perror("Open");
		_exit(-1);
	}
	/*
	 * Creating a shared mapping of file. Any updates to this are reflected
	 * to all process sharing this mapping, the child process in this case.
	 */
	if (fstat(fd, &stat_buf) < 0) {
                perror("fstat");
                _exit(-1);
        }
        size_file = stat_buf.st_size;

	if ((buff = mmap(NULL, size_file, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))
			== MAP_FAILED) {
		perror("mmap");
		_exit(-1);
	}

	pid = fork();
	if (pid < 0) {
		perror("Fork");
		_exit(-1);
	} else if (pid == 0) {
		printf("CHILD %c%c%c\n", *buff, *(buff+1), *(buff+2));
		buff[0] = 'D';
		printf("CHILD %c%c%c\n", *buff, *(buff+1), *(buff+2));
		if (msync(buff, size_file, MS_SYNC) < 0) {
			perror("Msync");
			_exit(-1);
		}
		_exit(0);
	} else {
		wait(-1);
		printf("PARENT %c%c%c\n", *buff, *(buff+1), *(buff+2));
	}
	if (munmap(buff, size_file) < 0) {
		perror("munmap");
		_exit(-1);
	}
	close(fd);	
	return 0;
}
