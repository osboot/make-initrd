#include <stdio.h>
#include <string.h>
#include <time.h>

int
main(int argc, char **argv)
{
	clockid_t clockid;
	int arg = 1;
	struct timespec tp = { 0 };

	if (arg == argc) {
		fprintf(stderr, "Usage: timestamp {monotonic|boottime} [raw|simple]\n");
		return 0;
	} else if (!strcmp(argv[arg], "monotonic")) {
		clockid = CLOCK_MONOTONIC_RAW;
	} else if (!strcmp(argv[arg], "boottime")) {
		clockid = CLOCK_BOOTTIME;
	} else {
		fprintf(stderr, "Unknown clockid: %s", argv[arg]);
		return 1;
	}
	arg++;

	clock_gettime(clockid, &tp);

	if (arg < argc) {
		if (!strcmp(argv[arg], "simple")) {
			printf("%06lu.%07lu", (unsigned long) tp.tv_sec, tp.tv_nsec);
			return 0;
		}
		if (!strcmp(argv[arg], "raw")) {
			printf("%lu.%lu", (unsigned long) tp.tv_sec, tp.tv_nsec);
			return 0;
		}
	}
	printf("[%5lu.%06lu] ", (unsigned long) tp.tv_sec, tp.tv_nsec / 1000);
	return 0;
}
