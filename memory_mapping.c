#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	char *buff, *shared_buff;

	if ((fd = open("testfile.txt", O_RDWR)) < 0) {
		perror("Open");
		_exit(-1);
	}
	/*
	 * Creating a private Copy-on-Write mapping of file. Any updates to this
	 * structure affects only the private memory and doesn't sync on the
	 * file.
	 */
	if ((buff = mmap(NULL, 512*1024, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0))
			== MAP_FAILED) {
		perror("mmap");
		_exit(-1);
	}

	printf("%c%c%c\n", *buff, *(buff + 1), *(buff + 2));
	buff[0] = 'S';
	printf("%c%c%c\n", *buff, *(buff + 1), *(buff + 2));
	/* Only memory value was changed, file doesn't get affected even if
	 * msync is tried
	 */
	/* Creating a shared mapping of file. Any changes made to this mapping,
	 * are reflected in all processes sharing this memory map. The changes
	 * affect the file as well. But msync should be called to make sure.
	 * Else all data are finally synced at munmap
	 */
	if ((shared_buff = mmap(NULL, 512*1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))
			== MAP_FAILED) {
		perror("mmap");
		_exit(-1);
	}
	printf("%c%c%c\n", *shared_buff, *(shared_buff + 1), *(shared_buff + 2));
	*(shared_buff + 1) = 'I';
	printf("%c%c%c\n", *shared_buff, *(shared_buff + 1), *(shared_buff + 2));

	if (msync(shared_buff, 512*1024, MS_SYNC) < 0) {
		perror("msync");
		_exit(-1);
	}
	if (munmap(shared_buff, 512*1024) < 0) {
		perror("Unmap");
		_exit(-1);
	}
	if (close(fd) < 0) {
		perror("Close");
		_exit(-1);
	}
	return 0;
}
