#include <stdio.h>
#include <string.h>
#include <time.h>

int
main(int argc, char **argv)
{
	struct timespec tp = { 0 };
	clock_gettime(CLOCK_MONOTONIC, &tp);
	if (argc == 2 && !strcmp(argv[1], "raw")) {
		printf("%06lu.%07lu", (unsigned long) tp.tv_sec, tp.tv_nsec);
		return 0;
	}
	printf("[%5lu.%06lu] ", (unsigned long) tp.tv_sec, tp.tv_nsec / 1000);
	return 0;
}
