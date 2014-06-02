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

	if (fstat(fd, &stat_buf) < 0) {
		perror("fstat");
		_exit(-1);
	}
	size_file = stat_buf.st_size;
	
	pid = fork();
	if (pid < 0) {
		perror("Fork");
		_exit(-1);
	} else if (pid == 0) {
		/* If the mapping is private, the child gets a copy-on-write
		 * mapping, ie it shares the same mapping until it starts
		 * writing in which case, it gets a private copy of that
		 * particular page
		 */
		/*
		 * Creating a private Copy-on-Write mapping of file. Any updates to this
		 * structure affects only the private memory and doesn't sync on the
		 * file.
		 */
		if ((buff = mmap(NULL, size_file, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0))
				== MAP_FAILED) {
			perror("mmap");
			_exit(-1);
		}
		printf("CHILD %c%c%c\n", *buff, *(buff+1), *(buff+2));
		buff[0] = 'D';
		printf("CHILD %c%c%c\n", *buff, *(buff+1), *(buff+2));
		_exit(0);
	} else {
		wait(-1);
		/* if the mapping is MAP_PRIVATE, then the changes the child
		 * made would be local to it's mapping and won't reflect in
		 * parent's mapping
		 */
		if ((buff = mmap(NULL, size_file, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0))
				== MAP_FAILED) {
			perror("mmap");
			_exit(-1);
		}
		printf("PARENT %c%c%c\n", *buff, *(buff+1), *(buff+2));
	}
	close(fd);
	if (munmap(buff, size_file) < 0) {
		perror("munmap");
		_exit(-1);
	}
	return 0;
}
