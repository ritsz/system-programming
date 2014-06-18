#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>

int handle_connection(int sockd)
{
	int ret, fd;
	char buff[2048];
	/* Read the name of file to read */
	if ((ret = read(sockd, buff, sizeof(buff))) < 0) {
		perror("socket read");
		exit(-1);
	}
	buff[ret] = '\0';

	printf("Request for %s file received\n", buff);

	if ((fd = open(buff, O_RDONLY)) < 0) {
		perror("file open");
		exit(-1);
	}

	if ((ret = read(fd, buff, 2048)) < 0) {
		perror("file read");
		exit(-1);
	}

	if ((ret = write(sockd, buff, ret)) < 0) {
		perror("socket write");
		exit(-1);
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0, cpid;
    struct sockaddr_in serv_addr, cli_addr; 

    char sendBuff[1025];
    time_t ticks; 

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		    perror("socket");
		    _exit(-1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
	int len = sizeof(cli_addr);    
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &len); 
	
	printf("connection accepted from %s, port %d\n",
			inet_ntop(AF_INET, &cli_addr.sin_addr, sendBuff,sizeof(sendBuff)),
			ntohs(cli_addr.sin_port));
	
	/* Adding support for concurrency. The connection that are accepted are
	 * handelled by a forked child. The parent only closes the connfd, thus
	 * bringing the reference count of the connfd to 1, which had become 2
	 * post forking. Thus when child calls close itself, connfd refcount
	 * goes to 0 and hence can be cleared. Also since reference count is 0,
	 * the kernel will send a FIN and initiate a TCP 4 way handshake.
	 */
	cpid = fork();
	if (cpid == 0) {
		close(listenfd);
		ticks = time(NULL);
        	snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        	write(connfd, sendBuff, strlen(sendBuff));
		if (handle_connection(connfd) < 0)
			perror("handle_connection");

		close(connfd);
		exit(0);
	}
        close(connfd);
     }
}
