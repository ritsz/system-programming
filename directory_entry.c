#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int main(void)
{
	/* dirent structure represents a directory entry, defined in dirent.h
	 * and includes inode d_ino, offset to next dirent d_off, length of
	 * record d_reclen, type of file d_type and file name d_name.
	 */
	struct dirent *entry;
	/* DIR represents a directory stream. It holds the directory file
	 * descriptor, some metadata and buffer that holds the directory
	 * entries.
	 */
	DIR *dir;

	dir = opendir("/root/PROGRAMMING/System_Programming");
	if (dir == NULL) {
		perror("opendir");
		exit(-1);
	}

	/* We can get the file descriptor that is backing the stream using dirfd
	 */
	printf("file descriptor of the directory is %d\n", dirfd(dir));

	while ((entry = readdir(dir)) != NULL) {
		printf("INODE : %u \t%s  [ %c ]\n", entry->d_ino, entry->d_name,
				entry->d_type);
	}

	closedir(dir);
	return 0;
}
