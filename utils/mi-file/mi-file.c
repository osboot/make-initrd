#define _GNU_SOURCE

#include <sys/param.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <error.h>
#include <errno.h>

#include <gelf.h>

static size_t read_bufsize = 4;
static int need_prefix = 1;
static char *separator = (char *) ":";

static void
show_help(int rc)
{
	printf("Usage: %s [OPTION...] [FILE...]\n"
	       "\n"
	       "Determine type of FILEs. This helper emulates a file.\n"
	       "It only detects symlinks, ELFs and scripts.\n"
	       "\n"
	       "Options:\n"
	       "      --help                 display this help and exit\n"
	       "  -v, --version              output version information and exit\n"
	       "  -b, --brief                do not prepend filenames to output lines\n"
	       "  -f, --files-from FILE      read the filenames to be examined from FILE\n"
	       "  -F, --separator STRING     use string as separator instead of `:'\n"
	       "  -n, --no-buffer            ignored\n"
	       "  -N, --no-pad               ignored\n"
	       "  -h, --no-dereference       ignored\n"
	       "  -L, --dereference          ignored\n"
	       "  -z, --uncompress           ignored\n"
	       "\n"
	       , program_invocation_short_name);
	exit(rc);
}

static void
print_version_and_exit(void)
{
	printf("%s version %s\n"
	       "\n"
	       "Written by Alexey Gladkov.\n"
	       "\n"
	       "Copyright (C) 2019  Alexey Gladkov <gladkov.alexey@gmail.com>\n"
	       "This is free software; see the source for copying conditions.  There is NO\n"
	       "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	       "\n"
	       , program_invocation_short_name, VERSION);
	exit(EXIT_SUCCESS);
}

static inline void
show_prefix(char *filename)
{
	if (need_prefix) {
		fputs(filename, stdout);
		fputs(separator, stdout);
		fputs(" ", stdout);
	}
}

static void
elf_file(int fd)
{
	int is_dynamic;
	Elf *e;
	Elf_Scn *scn;
	size_t shstrndx;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		error(EXIT_FAILURE, 0, "elf_begin: %s", elf_errmsg(-1));

	switch (gelf_getclass(e)) {
		case ELFCLASS32:
			fputs(" 32-bit", stdout);
			break;
		case ELFCLASS64:
			fputs(" 64-bit", stdout);
			break;
		default:
			fputs(" Invalid", stdout);
	}

	switch (elf_kind(e)) {
		case  ELF_K_NONE:
			fputs(", data", stdout);
			goto end;
		case  ELF_K_AR:
			fputs(", AR archive", stdout);
			goto end;
		case ELF_K_COFF:
			fputs(", COFF", stdout);
			goto end;
		case  ELF_K_ELF:
			fputs(", object", stdout);
			break;
		default:
			fputs(", unrecognized", stdout);
			goto end;
	}

	if (elf_getshdrstrndx(e, &shstrndx) != 0)
		error(EXIT_FAILURE, 0, "elf_getshdrstrndx: %s", elf_errmsg(-1));

	is_dynamic = 0;

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr  shdr;

		if (gelf_getshdr(scn , &shdr) != &shdr)
			error(EXIT_FAILURE, 0, "gelf_getshdr: %s", elf_errmsg(-1));

		if (shdr.sh_type == SHT_DYNAMIC) {
			is_dynamic = 1;
			break;
		}
	}

	if (is_dynamic)
		fputs(", dynamically linked", stdout);
	else
		fputs(", statically linked", stdout);

end:
	elf_end(e);
}

static void
determine_type(char *filename)
{
	int fd = -1;
	char buf[read_bufsize];

	fd = open(filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0) {
		if (errno == ELOOP) {
			show_prefix(filename);
			fputs("symbolic link\n", stdout);
		} else {
			error(0, errno, "open: %s", filename);
		}
		goto end;
	}

	if (pread(fd, buf, sizeof(buf), 0) < 0) {
		error(0, errno, "read: %s", filename);
		goto end;
	}

	if (buf[0] == '#' &&
	    buf[1] == '!') {
		show_prefix(filename);
		fputs("SHEBANG script text\n", stdout);
		goto end;
	}

	if (buf[0] == ELFMAG[0] &&
	    buf[1] == ELFMAG[1] &&
	    buf[2] == ELFMAG[2] &&
	    buf[3] == ELFMAG[3]) {
		show_prefix(filename);
		fputs("ELF", stdout);
		elf_file(fd);
		fputs("\n", stdout);
		goto end;
	}

	show_prefix(filename);
	fputs("UNKNOWN\n", stdout);
end:
	fflush(stdout);
	if (fd >= 0)
		close(fd);
}

int
main(int argc, char **argv)
{
	static struct option long_options[] = {
		{"brief", no_argument, 0, 'b' },
		{"dereference", no_argument, 0, 'L' },
		{"files-from", required_argument, 0, 'f' },
		{"help", no_argument, 0, 1 },
		{"no-buffer", no_argument, 0, 'n' },
		{"no-dereference", no_argument, 0, 'h' },
		{"no-pad", no_argument, 0, 'N' },
		{"separator", required_argument, 0, 'F' },
		{"uncompress", no_argument, 0, 'z'},
		{"version", no_argument, 0, 'v' },
		{0, 0, 0, 0 },
	};
	int c;
	int option_index = 0;
	int use_stdin = 0;

	while ((c = getopt_long(argc, argv, "nbhf:F:vLNz", long_options, &option_index)) != -1) {
		switch (c) {
			case 'b':
				need_prefix = 0;
				break;
			case 'f':
				use_stdin = (!strcmp(optarg, "-"));
				break;
			case 'F':
				separator = optarg;
				break;
			case 'v':
				print_version_and_exit();
				break;
			case 1:
				show_help(0);
				break;
			default:
				break;
		}
	}

	if (elf_version(EV_CURRENT) == EV_NONE)
		error(EXIT_FAILURE, 0, "ELF library initialization failed: %s", elf_errmsg(-1));

	if (use_stdin) {
		while (1) {
			char filename[MAXPATHLEN];

			errno = 0;
			if (!fgets(filename, sizeof(filename), stdin)) {
				if (feof(stdin))
					break;
				error(EXIT_FAILURE, errno, "fgets");
			}

			size_t len = strlen(filename);

			if (!len)
				continue;

			if (filename[len - 1] == '\n')
				filename[len - 1] = '\0';

			determine_type(filename);
		}

	} else {
		for (int i = optind; i < argc; i++)
			determine_type(argv[i]);
	}

	return EXIT_SUCCESS;
}
