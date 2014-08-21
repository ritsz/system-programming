#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


// Simple structure to keep track of the handle, and
// of what needs to be freed later.
typedef struct {
	int socket;
    	SSL *sslHandle;
    	SSL_CTX *sslContext;
} connection;

// For this example, we'll be testing on openssl.org
#define SERVER  "www.lwn.net"
#define PORT 443


// Establish a regular tcp connection
int tcpConnect ()
{
  	int error, handle;
  	struct hostent *host;
  	struct sockaddr_in server;

  	if (!(host = gethostbyname (SERVER))) {
		printf("Can't get hostname\n");
		return 0; /*return zero handle*/
	}

  	/* Create a socket */
	handle = socket (AF_INET, SOCK_STREAM, 0);
  	if (handle == -1 ) {
      		perror ("Socket");
      		handle = 0;
    	} else {
		/* Create a sockaddr structure with details about the host to
		 * connect to.
		 */
      		server.sin_family = AF_INET;
      		server.sin_port = htons (PORT);
      		server.sin_addr = *((struct in_addr *) host->h_addr);
      		bzero (&(server.sin_zero), 8);
		
		/* Connect to the specified host using the created socket */
      		error = connect (handle, (struct sockaddr *) &server,
                       	sizeof (struct sockaddr));
      		if (error == -1) {
          		perror ("Connect");
          		handle = 0;
        	}
    	}

  	return handle;
}

// Establish a connection using an SSL layer
connection *sslConnect (void) {
  	connection *c;

  	c = malloc (sizeof (connection));
  	c->sslHandle = NULL;
  	c->sslContext = NULL;

	/* Set up a TCP session first. Once the TCP is up, we can establish a
	 * SSL encrypted connection on top of that TCL connection. We use the
	 * structure declared above to save TCP socket, sslContext and
	 * sslHandle.
	 * 	1> Declare a new context
	 *	2> Create a new ssl handle in that context
	 *	3> Attach ssl handle to the tcp socket
	 *	4> Initiate SSL handshake.
	 */
  	c->socket = tcpConnect();
  	if (c->socket) {
      		// Register the error strings for libcrypto & libssl
      		SSL_load_error_strings ();
      		// Register the available ciphers and digests
      		SSL_library_init ();

      		// New context saying we are a client, and using SSL 2 or 3
      		c->sslContext = SSL_CTX_new (SSLv23_client_method ());
      		if (c->sslContext == NULL)
        		ERR_print_errors_fp (stderr);

      		/* Create an SSL struct for the connection using the client
		 * context we just created.
		 */
      		c->sslHandle = SSL_new (c->sslContext);
     		if (c->sslHandle == NULL)
       	 		ERR_print_errors_fp (stderr);

      		// Connect the SSL struct to our connection
      		if (!SSL_set_fd (c->sslHandle, c->socket))
        		ERR_print_errors_fp (stderr);

     		// Initiate SSL handshake
      		if (SSL_connect (c->sslHandle) != 1)
        		ERR_print_errors_fp (stderr);

		printf("SSL Handshake established. Data will now be encrypted\n");
    	} else {
      		printf("TCP connection failed.\n");
    	}

  return c;
}

// Disconnect & free connection struct
void sslDisconnect (connection *c)
{
  	if (c->socket)
    		close (c->socket);
  	if (c->sslHandle) {
      		SSL_shutdown (c->sslHandle);
      		SSL_free (c->sslHandle);
    	}
  	if (c->sslContext)
    		SSL_CTX_free (c->sslContext);

  	free (c);
}

// Read all available text from the connection
char *sslRead (connection *c)
{
  	const int readSize = 1024;
  	char *rc = NULL;
  	int received, count = 0;
  	char buffer[1024];

  	if (c) {
      		while (1) {
			/* Allocate 1024 bytes to rc or add 1024 bytes if rc not NULL
			 */
          		if (!rc)
            			rc = malloc (readSize * sizeof (char) + 1);
          		else
            			rc = realloc (rc, (count + 1)*readSize*sizeof(char)+1);

          		/* Read data on the sslHandle and save to a buffer */
			received = SSL_read (c->sslHandle, buffer, readSize);
          		buffer[received] = '\0';
			
			/* Add buffer to end of rc and if read size if less than
			 * 1024, then EOF must have been reached.
			 */
          		if (received > 0)
            			strcat (rc, buffer);
       			if (received < readSize)
     				break;
      	 		
			count++;
        	}
    	}

  	return rc;
}

// Write text to the connection
void sslWrite (connection *c, char *text)
{
  	if (c)
    		SSL_write (c->sslHandle, text, strlen (text));
}

// Very basic main: we send GET / and print the response.
int main (int argc, char **argv)
{
  	connection *c;
  	char *response;

  	c = sslConnect();
	if (c->sslHandle) {
		sslWrite (c, "GET /\r\n\r\n");
  		response = sslRead (c);

		printf ("%s\n", response);
	
  		sslDisconnect (c);
  		free (response);
	}
	
  	return 0;
}

