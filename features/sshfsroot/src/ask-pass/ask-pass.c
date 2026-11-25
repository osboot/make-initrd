// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

#include "cleanup.h"

void get_password(char **password, FILE *stream) _wronly_ptr_(1) _nonnull_(2);

int main(int argc, char *argv[])
{
	char *password _free_(free) = NULL;
	FILE *console _free_(fclose) = NULL;

	fclose(stdin);
	console = fopen("/dev/console", "a+");

	if (console == NULL) {
		fprintf(stderr, "ask-pass error: Can't open /dev/console: %s\n", strerror(errno));
		return 1;
	}

	if (argc > 1) {
		fprintf(console, "%s\n", argv[1]);
	}

	get_password(&password, console);

	if (password == NULL)
		return 1;

	printf("%s", password);

	return 0;
}

void get_password(char **password, FILE *stream)
{
	static struct termios oldt, newt;
	size_t n;
	int fd = fileno(stream);

	/*saving the old settings of stream and copy settings for resetting*/

	tcgetattr(fd, &oldt);
	newt = oldt;

	/*setting the approriate bit in the termios struct*/
	newt.c_lflag &= ~(ECHO);

	/*setting the new bits*/
	tcsetattr(fd, TCSANOW, &newt);

	/*reading the password from the console*/
	*password = NULL;

	if (getline(password, &n, stream) == -1) {
		fprintf(stream, "ask-pin error: Can't read password from stream: %s\n", strerror(errno));
		*password = mfree(*password);
	}

	/*resetting our old stream*/
	tcsetattr(fd, TCSANOW, &oldt);
}

