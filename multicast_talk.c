#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 16
#define MULTICAST_GROUP 3
#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr; 
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main()
{
	printf("CURRENT PID : %d\n", getpid());

	if ( (sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER)) < 0) {
		perror("socket");
		return -1;
	}

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

	int pid;
	printf("Enter PID to talk to : ");
	scanf("%d", &pid);

        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = pid; /* For Linux Kernel */
        dest_addr.nl_groups = (1 << MULTICAST_GROUP) ; /* unicast */

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

        char buffer[1024];
	printf("Enter data to send : ");
	scanf("%s", buffer);

	strcpy(NLMSG_DATA(nlh), buffer);

        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        printf("Sending message to %d\n", pid);
        sendmsg(sock_fd, &msg, 0);
        printf("Waiting for message from %d\n", pid);

        /* Read message from kernel */
      	recvmsg(sock_fd, &msg, 0);
      	printf("Received message payload: %s\n", NLMSG_DATA(nlh));
        close(sock_fd);

	return 0;
}
