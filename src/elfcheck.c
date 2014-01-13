#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <err.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

static int show_filenames = 0;

static const char cmdopts_s[] = "fVh";
static const struct option cmdopts[] = {
	{"filename",  no_argument, 0, 'f'},
	{"version",   no_argument, 0, 'V'},
	{"help",      no_argument, 0, 'h'},
	{NULL, 0, 0, 0}
};


static void __attribute__ ((noreturn))
print_help(const char *progname)
{
	printf("Usage: %s [options] [-|filename]...\n"
		"\n"
		"Displays the fields from ELF header.\n"
		"\n"
		"Options:\n"
		"   -f, --filename     Show only name of ELF files;\n"
		"   -V, --version      Show version of program and exit;\n"
		"   -h, --help         Show this text and exit.\n"
		"\n",
		progname);
	exit(EXIT_SUCCESS);
}

static void __attribute__ ((noreturn))
print_version(const char *progname)
{
	printf("%s version " VERSION "\n"
	       "Written by Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "\n"
	       "Copyright (C) 2012  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n",
	       progname);
	exit(EXIT_SUCCESS);
}


static void
elfcheck(const char *fn)
{
	int fd;
	unsigned char buf[32];
	struct stat st;

	// Ignore nonexistent file and not regular file
	if (stat(fn, &st) < 0 || !S_ISREG(st.st_mode))
		return;

	if ((fd = open(fn, O_RDONLY, 0)) < 0) {
		err(EXIT_FAILURE, "open `%s\" failed", fn);
	}

	if (read(fd, &buf, sizeof(buf)) < 0)
		err(EXIT_FAILURE, "read `%s\" failed", fn);

	close(fd);

	if (buf[0] == 0x7F &&
	    buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
		if (show_filenames) {
			printf("%s\n" , fn);
			return;
		}
		printf("%s\tELF, ", fn);
	} else {
		if (!show_filenames)
			printf("%s\tdata\n", fn);
		return;
	}

	switch (buf[4]) {
		case 0: printf("invalid class, "); break;
		case 1: printf("32-bit, "); break;
		case 2: printf("64-bit, "); break;
	}

	switch (buf[5]) {
		case 0: printf("invalid byte order, "); break;
		case 1: printf("LSB, "); break;
		case 2: printf("MSB, "); break;
	}

	printf("version %u, ", buf[6]);

	switch (buf[7]) {
		case 0: printf("SYSV, ");         break;
		case   1: printf("HPUX, ");       break;
		case   2: printf("NETBSD, ");     break;
		case   3: printf("LINUX, ");      break;
		case   6: printf("SOLARIS, ");    break;
		case   7: printf("AIX, ");        break;
		case   8: printf("IRIX, ");       break;
		case   9: printf("FREEBSD, ");    break;
		case  10: printf("TRU64, ");      break;
		case  11: printf("MODESTO, ");    break;
		case  12: printf("OPENBSD, ");    break;
		case  97: printf("ARM, ");        break;
		case 255: printf("STANDALONE,");  break;
	}

	switch (buf[16]) {
		case 0: printf("no file type");  break;
		case 1: printf("relocatable");   break;
		case 2: printf("executable");    break;
		case 3: printf("shared object"); break;
		case 4: printf("core file");     break;
		default:
			printf("unknown type");
	}
	printf("\n");
}

int
main(int argc, char **argv)
{
	int c;

	char *line = NULL;
	size_t len = 0;
	ssize_t nr = 0;

	while ((c = getopt_long(argc, argv, cmdopts_s, cmdopts, NULL)) != -1) {
		switch (c) {
			case 'f':
				show_filenames = 1;
				break;
			case 'V':
				print_version(basename(argv[0]));
			case 'h':
				print_help(basename(argv[0]));
		}
	}

	if (optind >= argc)
		errx(EXIT_FAILURE, "ERROR: More arguments required.");

	if (!strcmp(argv[optind], "-")) {
		while ((nr = getline(&line, &len, stdin)) != -1) {
			line[nr-1] = '\0';
			elfcheck(line);
		}
		free(line);
		optind++;
	}

	for (; optind < argc; optind++)
		elfcheck(argv[optind]);

	return EXIT_SUCCESS;
}
