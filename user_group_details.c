#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	uid_t uid = getuid();
	uid_t eid = geteuid();

	printf("%s was run with %u UID and %u EUID\n", argv[0], uid, eid);
	/* The permission on the directory is 710 for root.root . Thus no one
	 * except the root should be able to list this directory.
	 */
	system("ls -l");
}
