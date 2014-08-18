#define _GNU_SOURCE /* enable some of the mmap flags */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int
main(int argc, char *argv[])
{
	void *where;
	size_t length = 4096;

	where = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS  | MAP_LOCKED | MAP_POPULATE, -1, 0);
	if (where != MAP_FAILED) {
		printf("Mapped at %p\n", where);
	} else {
		perror("Mapping failed");
	}
	return EXIT_SUCCESS;
}

