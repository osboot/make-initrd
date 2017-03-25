#ifndef _UEVENT_WORKER_H_
#define _UEVENT_WORKER_H_

#include <unistd.h>

pid_t spawn_worker(char *path, char **argv);

#endif /* _UEVENT_WORKER_H_ */
