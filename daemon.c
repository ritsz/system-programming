#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>

int main(void)
{
	pid_t pid;
	int i;

	pid = fork();
	if (pid < 0) {
		perror("Fork");
		_exit(-1);
	} else if (pid > 0)
		_exit(0);	/* Exit and end the parent. Init process will
				   take up the orphaned child, which is required
				   for daemons, as init is always a daemon's
				   parent */
	
	/* Change the session ID of the process */
	if (setsid() == -1) {
		perror("Setsid");
		return -1;
	}

	/* Set working directory as something that cannot be unmounted, like /
	 * */
	if (chdir("/") == -1) {
		perror("chdir");
		return -1;
	}

	/* Open a file for logging */

	i = open("/root/PROGRAMMING/System_Programming/daemon_log",
			O_RDWR|O_CREAT);

	/* Close all open file descriptors, STDIN and STDOUT in our case
	   */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);

	dup(i);
	dup(i);

	while (1) {
		write(STDOUT_FILENO, "Logging Begins\n", 15);
		sleep(5);
	}

}
