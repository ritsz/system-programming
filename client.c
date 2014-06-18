#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

int main(int argc, char *argv[])
{
	int sockfd = 0, n = 0;
	char recvBuff[2048];
	struct sockaddr_in serv_addr; 

	if(argc != 2) {
		printf("\n Usage: %s <ip of server>\n",argv[0]);
		return 1;
	} 

	memset(recvBuff, '0',sizeof(recvBuff));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Error : Could not create socket \n");
		return 1;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) {
		printf("\n inet_pton error occured\n");
		return 1;
	} 

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n Error : Connect Failed \n");
		return 1;
	} 

	/* Timestamp received */

	n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
	if (n < 0)
		perror("socket read");
	recvBuff[n] = 0;
	printf("%s\n", recvBuff);


	while (1) {
		char filename[128] = {0};
		printf("\n\nEnter a file name : ");
		scanf("%s", filename);
		if (filename[0] == '?') {
			close(sockfd);
			return 0;
		}
		if ((write(sockfd, filename, 128)) < 0) {
			perror("Socket write");
			close(sockfd);
			exit(-1);
		}

		memset(recvBuff, 0, sizeof(recvBuff));
		n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
		if (n < 0)
			perror("socket read");
		recvBuff[n] = 0;
		printf("%s\n", recvBuff);
	}
	return 0;
}
