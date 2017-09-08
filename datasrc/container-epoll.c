#include <sys/epoll.h>

#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include "container.h"

int
epollin_init(void)
{
	int fd;

	if ((fd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		error(EXIT_FAILURE, errno, "epoll_create1");

	return fd;
}

void
epollin_add(int fd_ep, int fd)
{
	struct epoll_event ev = {};

	ev.events  = EPOLLIN;
	ev.data.fd = fd;

	if (epoll_ctl(fd_ep, EPOLL_CTL_ADD, fd, &ev) < 0)
		error(EXIT_FAILURE, errno, "epoll_ctl");
}

void
epollin_remove(int fd_ep, int fd)
{
	if (fd < 0)
		return;

	if (epoll_ctl(fd_ep, EPOLL_CTL_DEL, fd, NULL) < 0)
		error(EXIT_FAILURE, errno, "epoll_ctl");

	close(fd);
}
