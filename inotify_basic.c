#include <stdio.h>
#include <sys/inotify.h>

int main(void)
{
	int fd, watch_d;
	char buf[512];
	if ((fd = inotify_init()) == -1)
		perror("Inotify Init");
	
	if ((watch_d = inotify_add_watch(fd,
			"/root/PROGRAMMING/System_Programming",
			IN_MOVED_TO|IN_MOVED_FROM|IN_CREATE|IN_DELETE)) == -1)
		perror("inotify_add_watch");

	while (1) {
		int len = read(fd, buf, 512);
		int i = 0;
		while(i<len) {
			struct inotify_event *event = (struct inotify_event *)&buf[i];
			printf("wd = %d maske = %d cookie  = %d len = %d dir = %s\n", event->wd,
					event->mask, event->cookie, event->len,
					(event->mask & IN_ISDIR) ? "yes" : "no");
			if (event->len)
				printf("Name = %s\n", event->name);
			
			if (event->mask & IN_DELETE) {
				printf("Delete Event. Stopping monitor\n");
				if (inotify_rm_watch(fd, watch_d) == -1)
					perror("Removing watch");
				if (close(fd) == -1)
					perror("Closing event FD");
				return(0);
			}

			i += sizeof(struct inotify_event) + event->len;
		}
	}
}
