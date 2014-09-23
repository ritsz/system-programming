#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 31
#define MULTICAST_GROUP 3
#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr; 
struct nlmsghdr *nlh;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main()
{

}
