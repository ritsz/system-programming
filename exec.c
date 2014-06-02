#include <unistd.h>

int main(void)
{
	char *arg_vec[] = {"/bin/ls", "-l", "-a", "-h", (char*)NULL};
	if (execve("/bin/ls", arg_vec, (char*)NULL) < 0)
		perror("Exec");
	return 0;
}
