#include <unistd.h>

int main(void)
{
	int pid;
	char *arg_vec[] = {"/bin/ls", "-l", "-a", "-h", (char*)NULL};

	pid = fork();

	if (pid < 0) {
		perror("Fork");
		_exit(-1);
	} else if (pid == 0) {
		if (execve("/bin/ls", arg_vec, (char*)NULL) < 0)
			perror("Exec");
	} else {
		system("ps -elf | egrep '(ls|a.out)'");
		printf("\n");
	/* 
	 *  0 S root      2539  1714  0  80   0 -   511 wait   00:06 pts/0    00:00:00 ./a.out
	 *  0 Z root      2540  2539  0  80   0 -     0 exit   00:06 pts/0    00:00:00 [ls] <defunct>
	 */
		wait(NULL);
	}
	return 0;
}
