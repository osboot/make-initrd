#include <stdio.h>
#include <time.h>

int
main(void)
{
	struct timespec tp = { 0 };
	clock_gettime(CLOCK_MONOTONIC, &tp);
	printf("[%5lu.%06lu] ", (unsigned long) tp.tv_sec, tp.tv_nsec / 1000);
	return 0;
}
