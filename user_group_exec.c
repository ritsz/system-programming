#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
	/* If the EUID is set, the priviledges are dropped temperorily and can
	 * be reclaimed if seteuid is called again. If the UID is set, the
	 * priviledges are dropped permanently and cannot be reclaimed. 
	 */
	if (seteuid(1000)) {
		perror("SETEUID");
	}

	pid_t pid = fork();

	if (pid < 0) {
		perror("Fork");
		exit(-1);
	} else if (pid == 0) {
		printf("CHILD UID %u and EUID %u\n", getuid(), geteuid());
		/* Changing the EUID and UID and testing what UID/EUID are
		 * present after exec and whether they are able to run certain
		 * priviledged commands.
		 */
		if (execl("user_group_details", "user_group_details", NULL))
			 perror("EXECL");
	} else {
		if (seteuid(0)) {
			perror("SETEUID");
		}
		printf("PARENT UID %u and EUID %u\n", getuid(), geteuid());
		wait(NULL);
	}
}
