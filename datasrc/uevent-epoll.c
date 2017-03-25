#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>

#include "uevent-logging.h"
#include "uevent-epoll.h"

void
epollin_add(int fd_ep, int fd)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(struct epoll_event));
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	if (epoll_ctl(fd_ep, EPOLL_CTL_ADD, fd, &ev) < 0)
		fatal("epoll_ctl: %m");
}

void
epollin_remove(int fd_ep, int fd)
{
	if (fd < 0)
		return;
	epoll_ctl(fd_ep, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}
