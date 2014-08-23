#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
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
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define RSA_SERVER_CERT "server.keycrt"
#define RSA_SERVER_KEY  "server.keycrt"

#define RSA_SERVER_CA_CERT "server.keycrt"
#define RSA_SERVER_CA_PATH "sys$common:[syshlp.examples.ssl]"

#define ON 1
#define OFF 0

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }


#define SERVER_PORT  12345

#define TRUE             1
#define FALSE            0

#define PIPING 0

void sigchld_handler(int sig)
{
	int status = 0;
	while(waitpid(-1, &status, WNOHANG) > 0)
		;
	
	if (WIFEXITED(status))
		printf("[%d] %d connection has been closed\n", getpid(), WEXITSTATUS(status));
	return;
}


int main (int argc, char *argv[])
{
	time_t ticks;
	int    i, len, rc, cpid, on = 1;
   	int    listen_sd, newfd;
   	struct sockaddr_in addr;
   	struct timeval timeout;
   	struct sockaddr_in serv_addr, cli_addr;
	fd_set master_set;
	int err;
	int pipefd[2];
	char sendBuff[1024];
	SSL_CTX *ctx;
        SSL *ssl;
        SSL_METHOD *meth;
        X509 *client_cert = NULL;
	
	memset(sendBuff, '0', sizeof(sendBuff)); 	 
	signal(SIGCHLD, sigchld_handler);

	rc = pipe(pipefd);
	if (rc < 0) {
		perror("pipe() ");
		exit(-1);
	}

	fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL)|O_NONBLOCK);
	fcntl(pipefd[1], F_SETFL, fcntl(pipefd[1], F_GETFL)|O_NONBLOCK);
	
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  	if (listen_sd < 0) {
      		perror("socket() failed");
      		exit(-1);
   	}

   	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
	if (rc < 0) {
      		perror("setsockopt() failed");
      		close(listen_sd);
      		exit(-1);
   	}

	memset(&addr, 0, sizeof(addr));
   	addr.sin_family      = AF_INET;
   	addr.sin_addr.s_addr = htonl(INADDR_ANY);
   	addr.sin_port        = htons(SERVER_PORT);
   	
	rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
   	if (rc < 0) {
      		perror("bind() failed");
      		close(listen_sd);
      		exit(-1);
   	}
	
	
	SSL_library_init();
	meth = SSLv3_method();
        ctx = SSL_CTX_new(meth);
        if (!ctx) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        if (SSL_CTX_use_certificate_file(ctx, RSA_SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                exit(1);
 	}

        if (SSL_CTX_use_PrivateKey_file(ctx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                exit(1);
        }

        if (!SSL_CTX_check_private_key(ctx)) {
                fprintf(stderr,"Private key does not match the certificate public key\n");
                exit(1);
        }
	
	ssl = SSL_new(ctx);
        RETURN_NULL(ssl);
	
	FD_ZERO(&master_set);
	FD_SET(listen_sd, &master_set);
	if (PIPING)
		FD_SET(pipefd[0], &master_set);
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;

	rc = listen(listen_sd, 32);
   	if (rc < 0) {
      		perror("listen() failed");
      		close(listen_sd);
      		exit(-1);
   	}
	
	 while(1) {
                int len = sizeof(addr);
                printf("[%d] Waiting on select\n", getpid());
		/* listen_sd will be the maximum as parent doesn't worry about
		 * the newfd, only the child handles it.
		 */
		rc = select(listen_sd + 1, &master_set, NULL, NULL, &timeout);
		if (rc < 0)
			perror("select()");

		if (FD_ISSET(listen_sd, &master_set)) {

			newfd = accept(listen_sd, (struct sockaddr*)&addr, &len);
			if (newfd == -1) {
				perror("accept");
				exit(-1);
			}
                	cpid = fork();
                	if (cpid == 0) {

                		close(listen_sd);
                        	/* Assign the socket into the SSL structure (SSL and socket without BIO) */
        			SSL_set_fd(ssl, newfd);
		
        			/* Perform SSL Handshake on the SSL server */
        			err = SSL_accept(ssl);
        			RETURN_SSL(err);
			
        			/* Informational output (optional) */
        			printf("[%d] SSL connection using %s\n", getpid(), SSL_get_cipher (ssl));
				ticks = time(NULL);
                        	snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
       				
				FD_ZERO(&master_set);
				FD_SET(newfd, &master_set);
				if (PIPING)
					FD_SET(pipefd[0], &master_set);
				
				do {
					rc = select(newfd + 1, &master_set, NULL, NULL, &timeout);
					if (rc <= 0)
						perror("select()");

					if (FD_ISSET(pipefd[0], &master_set) && PIPING ) {
						/* Data from parent, needs to be
						 * forwarded to clients.
						 */
						printf("[%d] Got data from PARENT process\n", getpid());
						int n_read = read(pipefd[0], sendBuff,strlen(sendBuff));
						/* Should not break if n_read is
						 * zero, since that means some
						 * othet task has ended.
						 */
						printf("[%d] Write to socket\n", getpid());
						SSL_write(ssl, sendBuff, n_read);
					}

					if (FD_ISSET(newfd, &master_set)) {
						printf("[%d] Got data on socket\n", getpid());
						int n_read = SSL_read(ssl, sendBuff, strlen(sendBuff));
						/* Exit thread if session closed */
						if (n_read <= 0)
							break;
						if (PIPING)
							write(pipefd[1], sendBuff, n_read);
						else
							SSL_write(ssl, sendBuff, n_read);
						printf("[%d] Data forwarded to parent\n", getpid());
					}
				} while(rc > 0);



				err = SSL_shutdown(ssl);
	        		RETURN_SSL(err);
		
	        		/* Terminate communication on a socket */
	        		err = close(newfd);
	        		RETURN_ERR(err, "close");
				exit(newfd);
			}
	                printf("[%d] %d connection accepted by [%d] from %s, port%d\n",
					getpid(), newfd, cpid,
	                        	inet_ntop(AF_INET, &cli_addr.sin_addr, sendBuff,sizeof(sendBuff)),
	                        	ntohs(cli_addr.sin_port));
			/* Parent close the connfd descriptor */
	                close(newfd);
		}/*End of FD_ISSET(listen_sd, ...);*/
		if (FD_ISSET(pipefd[0], &master_set) && PIPING) {
			/*data available to parent from child*/
			printf("[%d] Child sending data\n", getpid());
			int n_read = read(pipefd[0], sendBuff, strlen(sendBuff));
			sendBuff[n_read] = '\0';
			/* Don't write closed session data to pipe */
			if (n_read > 0)
				write(pipefd[1], sendBuff, n_read);
			printf("[%d] Data written to pipe for CHILD process to handle\n", getpid());
		}
	}

	/* Free the SSL structure */
        SSL_free(ssl);

        /* Free the SSL_CTX structure */
        SSL_CTX_free(ctx);

	return 0;
}
