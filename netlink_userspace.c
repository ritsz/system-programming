#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr; /*Special netlink protocol socket
					  address structure */
struct nlmsghdr *nlh;
struct iovec iov;
int sock_fd;
struct msghdr msg;

void main()
{
	nlh = NULL;
    	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    	if (sock_fd < 0) {
        	perror("socket");	
		return -1;
	}
	/* All field in the struture except family and local pid need to be put
	 * zero. That includes the multicast bits. The local pid and address
	 * family act as addresses.
	 */
    	memset(&src_addr, 0, sizeof(src_addr));
    	src_addr.nl_family = AF_NETLINK;
    	src_addr.nl_pid = getpid(); /* self pid */
	
	/* Bind the netlink socket adress  structure with the netlink family
	 * socket fd created earlier.
	 */
    	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    	/* Again, the destination address structure is made zero. Only family
	 * and pid (zero to talk to kernel) are specified.
	 */
	memset(&dest_addr, 0, sizeof(dest_addr));
    	dest_addr.nl_family = AF_NETLINK;
    	dest_addr.nl_pid = 0; /* For Linux Kernel */
    	dest_addr.nl_groups = 0; /* unicast */

    	/* The netlink message header changes for each data transfer. It holds
	 * the data to be sent, the total size of data plus header size. The
	 * header is required to have some common ground for netlink messages of
	 * all protocols.
	 */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    	nlh->nlmsg_pid = getpid();
    	nlh->nlmsg_flags = 0;

    	strcpy(NLMSG_DATA(nlh), "Hello");
	
    	iov.iov_base = (void *)nlh;
    	iov.iov_len = nlh->nlmsg_len;
    	msg.msg_name = (void *)&dest_addr;
    	msg.msg_namelen = sizeof(dest_addr);
    	msg.msg_iov = &iov;
    	msg.msg_iovlen = 1;
	
    	printf("Sending message to kernel\n");
    	sendmsg(sock_fd, &msg, 0);
    	printf("Waiting for message from kernel\n");

    	/* Read message from kernel */
    	recvmsg(sock_fd, &msg, 0);
    	printf("Received message payload: %s\n", NLMSG_DATA(nlh));
    	close(sock_fd);
}
