#include <stdio.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

int read_inotify_queue(char buf[], int len)
{
	int i = 0;
	while (i<len) {
		struct inotify_event *event = (struct inotify_event *)&buf[i];
                printf("wd = %d maske = %d cookie  = %d len = %d dir = %s\n", event->wd,
                	event->mask, event->cookie, event->len,
                	(event->mask & IN_ISDIR) ? "yes" : "no");
                if (event->len)
                	printf("Name = %s\n", event->name);
		
		i += sizeof(struct inotify_event) + event->len;
	}
	return 0;
}


int main(void)
{
	/* Making the inotify instance file descriptors O_NONBLOCK and going to
	 * multiplex over them using select(). The program will block on
	 * select() but the read from fd won't block
	 */
	int fd, watch_d, fd2, watch_d2, flags, len;
	char buf[512];
	if ((fd = inotify_init()) == -1) {
		perror("Inotify Init");
		_exit(-1);
	}
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);

	if ((watch_d = inotify_add_watch(fd,
			"/root/PROGRAMMING/System_Programming",
			IN_MOVED_TO|IN_MOVED_FROM|IN_CREATE|IN_DELETE)) == -1) {
		perror("inotify_add_watch");
		_exit(-1);
	}
	if ((fd2 = inotify_init()) == -1) {
		perror("Inotify Init");
		_exit(-1);
	}
	flags = fcntl(fd2, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd2, F_SETFL, flags);

	if ((watch_d2 = inotify_add_watch(fd2,
			"/root/PROGRAMMING/Scripting",
			IN_MOVED_TO|IN_MOVED_FROM|IN_CREATE|IN_DELETE)) == -1) {
		perror("inotify_add_watch");
		_exit(-1);
	}
	/* We will select() over all the file descriptors*/
	while (1) {
		fd_set readfds, writefds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(fd, &readfds);
		FD_SET(fd2, &readfds);
		/* Moving a file between the two watched directories produces 2
		 * events, one for each watch. The way we can know that these
		 * events are related is that they have the same cookie value
		 */

		write(STDOUT_FILENO, "\nWaiting on select()\n", 21);
		int ret_fd = select(max(fd,fd2)+1, &readfds, &writefds,NULL, NULL);
		if (ret_fd == -1) {
			perror("select");
			_exit(-1);
		}
		if (FD_ISSET(fd, &readfds)) {
			if ((len = read(fd, buf, 512)) == -1) {
				perror("Read");
				_exit(-1);
			}
			read_inotify_queue(buf, len);	
		}
		if (FD_ISSET(fd2, &readfds)) {
			if ((len = read(fd2, buf, 512)) == -1) {
				perror("Read");
				_exit(-1);
			}
			read_inotify_queue(buf, len);	
		}
	}	
}
