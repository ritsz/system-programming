#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <signal.h>
#include <stdio.h>

void mysigint()
{
	write(STDOUT_FILENO, "Caught the SIGINT signal!\n", 26);
	return;    
} 

/* Our own SIGKILL handler */
void mysigkill()
{
	write(STDOUT_FILENO, "Caught the SIGKILL signal!\n", 27);
	return;    
} 

/* Our own SIGHUP handler */
void mysighup()
{
	write(STDOUT_FILENO, "Caught the SIGHUP signal!\n", 26);
	return;    
} 

/* Our own SIGTERM handler */
void mysigterm()
{
	write(STDOUT_FILENO, "Caught the SIGTERM signal!\n", 27);
	return;    
}

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

	/* Duplicate the file fd over standard input, output and error */
	dup(i);
	dup(i);
	dup(i);
	/* Add signal handlers */
	if (signal(SIGINT, mysigint) == SIG_ERR)
       		perror("Cannot handle SIGINT!");        
    	if (signal(SIGHUP, mysighup) == SIG_ERR)
       		perror("Cannot handle SIGHUP!");  
    	if (signal(SIGTERM, mysigterm) == SIG_ERR)
       		perror("Cannot handle SIGTERM!"); 
	if (signal(SIGKILL, mysigterm) == SIG_ERR)
       		perror("Cannot handle SIGKILL!"); 

	while (1) {
		write(STDOUT_FILENO, "Logging Daemon\n", 15);
		sleep(5);
	}
}
