// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <err.h>

#include <gelf.h>

#include "queue.h"
#include "tree.h"
#include "enqueue.h"
#include "elf_dlopen.h"

#define _FDO_ELF_METADATA 0x407c0c0a

extern int verbose;

static bool is_dynamic_elf_file(const char *filename, int fd) __attribute__((__nonnull__ (1)));
static int enqueue_elf_dlopen(const char *filename, int fd) __attribute__((__nonnull__ (1)));
static int enqueue_shared_libraries(const char *filename) __attribute__((__nonnull__ (1)));


bool is_dynamic_elf_file(const char *filename, int fd)
{
	bool is_dynamic = false;
	Elf *e;
	Elf_Scn *scn;
	size_t shstrndx;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("%s: elf_begin: %s", filename, elf_errmsg(-1));
		goto err;
	}

	switch (elf_kind(e)) {
		case  ELF_K_NONE:
		case  ELF_K_AR:
		case ELF_K_COFF:
		case  ELF_K_ELF:
			break;
		default:
			goto end;
	}

	if (elf_getshdrstrndx(e, &shstrndx) != 0) {
		warnx("%s: elf_getshdrstrndx: %s", filename, elf_errmsg(-1));
		goto end;
	}

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr  shdr;

		if (gelf_getshdr(scn, &shdr) != &shdr) {
			warnx("%s: gelf_getshdr: %s", filename, elf_errmsg(-1));
			goto end;
		}

		if (shdr.sh_type == SHT_DYNAMIC) {
			is_dynamic = true;
			break;
		}
	}
end:
	elf_end(e);
err:
	return is_dynamic;
}

int enqueue_elf_dlopen(const char *filename, int fd)
{
	int ret = 0;
	char library[PATH_MAX + 1];
	Elf *e;
	Elf_Scn *scn;

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("%s: elf_begin: %s", filename, elf_errmsg(-1));
		goto err;
	}

	switch (elf_kind(e)) {
		case  ELF_K_NONE:
		case  ELF_K_AR:
		case ELF_K_COFF:
		case  ELF_K_ELF:
			break;
		default:
			goto end;
	}

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr  shdr;
		GElf_Nhdr nhdr;
		size_t off, next, n_off, d_off;
		Elf_Data *data;

		if (gelf_getshdr(scn, &shdr) != &shdr) {
			warnx("%s: gelf_getshdr: %s", filename, elf_errmsg(-1));
			goto end;
		}

		if (shdr.sh_type != SHT_NOTE)
			continue;

		data = elf_getdata(scn, NULL);
		if (data == NULL) {
			warnx("%s: elf_getdata: %s", filename, elf_errmsg(-1));
			goto end;
		}

		off = 0;
		while ((next = gelf_getnote(data, off, &nhdr, &n_off, &d_off)) > 0) {
			if (nhdr.n_type == _FDO_ELF_METADATA && nhdr.n_namesz == sizeof(ELF_NOTE_FDO) && !strcmp(data->d_buf + n_off, ELF_NOTE_FDO)) {
				library[0] = '\0';

				process_json_metadata(filename, (char *)data->d_buf + d_off, library);

				if (library[0] == '/' && !is_path_added(library))
					enqueue_item(library, -1);
			}
			off = next;
		}
	}
end:
	elf_end(e);
err:
	return ret;
}

int enqueue_shared_libraries(const char *filename)
{
	FILE *pfd;
	char *line = NULL;
	size_t len = 0;
	ssize_t n;
	char command[PATH_MAX + 10];

	snprintf(command, sizeof(command), "ldd %s 2>&1", filename);

	pfd = popen(command, "r");
	if (!pfd) {
		warn("popen(ldd): %s", filename);
		return -1;
	}

	while ((n = getline(&line, &len, pfd)) != -1) {
		char *p;

		if (line[n - 1] == '\n')
			line[n - 1] = '\0';

		p = strstr(line, "(0x");
		if (!p)
			continue;
		*p-- = '\0';

		while (line != p && isspace(*p))
			*p-- = '\0';

		p = strstr(line, " => ");
		if (p)
			p += 4;
		else
			p = line;

		while (*p != '\0' && isspace(*p))
			*p++ = '\0';

		if (*p != '/')
			continue;

		if (verbose > 1)
			warnx("shared object '%s' depends on '%s'", filename, p);

		if (is_path_added(p))
			continue;

		enqueue_item(p, -1);
	}

	free(line);
	pclose(pfd);

	return 0;
}

void init_elf_library(void)
{
	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));
}

bool is_elf_file(char buf[LINE_MAX])
{
	return (buf[0] == ELFMAG[0] &&
	        buf[1] == ELFMAG[1] &&
	        buf[2] == ELFMAG[2] &&
	        buf[3] == ELFMAG[3]);
}

int enqueue_libraries(const char *filename, int fd)
{
	int ret = 0;

	if (is_dynamic_elf_file(filename, fd)) {
		ret = enqueue_shared_libraries(filename);
		if (!ret)
			ret = enqueue_elf_dlopen(filename, fd);
	}
	return ret;
}
