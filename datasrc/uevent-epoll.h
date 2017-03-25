#ifndef _UEVENT_EPOLL_H_
#define _UEVENT_EPOLL_H_

#include <sys/epoll.h>

void epollin_add(int fd_ep, int fd);
void epollin_remove(int fd_ep, int fd);

#endif /* _UEVENT_EPOLL_H_ */
