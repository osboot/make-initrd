/*
 * Handle autoconfiguration of md devices.
 *
 * Processes "md=..." and "raid=..." parameters passed as separate
 * command arguments, ignores other parameters.
 */

#include <sys/stat.h>
#include <unistd.h>
#include "kinit.h"
#include "do_mounts.h"

/* Create the device node "name" */
int create_dev(const char *name, dev_t dev)
{
	unlink(name);
	return mknod(name, S_IFBLK | 0600, dev);
}

int main(int argc, char *argv[])
{
	md_run(argc - 1, argv + 1);
	return 0;
}
