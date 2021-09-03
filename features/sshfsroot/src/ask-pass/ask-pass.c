#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

void get_password(char **password, FILE *stream);

int main(int argc, char *argv[])
{
	char *password;
	FILE *console;

	fclose(stdin);
	console=fopen("/dev/console", "a+");

	if (console == NULL) {
		fprintf(stderr, "ask-pass error: Can't open /dev/console: %s\n", strerror(errno));
		return 1;
	}

	if (argc > 1) {
		fprintf(console, "%s\n", argv[1]);
	}

	get_password(&password, console);

	fclose(console);

	if (password == NULL)
		return 1;

	printf("%s", password);
	free(password);

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
		free(*password);
		*password=NULL;
	}

	/*resetting our old stream*/
	tcsetattr(fd, TCSANOW, &oldt);
}

