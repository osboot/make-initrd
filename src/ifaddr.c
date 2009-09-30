#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int fd, iname = 1;
	struct ifreq ifr;

	if (argc == 3) {
		if (argv[1][0] != '-' || argv[1][1] != 'u' || argv[1][2] != '\0')
			goto usage;
		iname = 2;
	} else if (argc == 1)
		goto usage;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		goto err;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, argv[iname], IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (iname == 2) {
		if(ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
			goto err;

		if (!(ifr.ifr_flags & IFF_UP))
			goto err;
	}

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
		goto err;

	fputs(inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr), stdout);
	fputs("\n", stdout);

	close(fd);
	return 0;

usage:	fputs("Usage: ifaddr [-U] <iface>\n", stdout);
	return 0;

err:	close(fd);
	return 1;
}
