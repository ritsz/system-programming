#include <execinfo.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

static void full_write(int fd, const char *buf, size_t len)
{
	while (len > 0) {
		ssize_t ret = write(fd, buf, len);
		if ((ret == -1) && (errno != EINTR))
			break;
                buf += (size_t) ret;
		len -= (size_t) ret;
	}
}

void print_backtrace(void)
{
	static const char __start[] = "BACKTRACE ------------\n";
	static const char __end[] = "----------------------\n";
	void *__bt[1024];
	int __bt_size;
	char **__bt_syms;
	int i;

	__bt_size = backtrace(__bt, 1024);
	__bt_syms = backtrace_symbols(__bt, __bt_size);
	full_write(STDERR_FILENO, __start, strlen(__start));
	for (i = 1; i < __bt_size; i++) {
		size_t len = strlen(__bt_syms[i]);
		full_write(STDERR_FILENO, __bt_syms[i], len);
		full_write(STDERR_FILENO, "\n", 1);
	}
	full_write(STDERR_FILENO, __end, strlen(__end));
	free(__bt_syms);
}
