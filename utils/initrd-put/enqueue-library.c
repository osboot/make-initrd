// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <search.h>
#include <sysexits.h>
#include <stdint.h>

#include <gelf.h>

#include "memory.h"
#include "queue.h"
#include "tree.h"
#include "enqueue.h"
#include "elf_dlopen.h"
#include "temp_failure_retry.h"

#ifndef NT_FDO_DLOPEN_METADATA
# define NT_FDO_DLOPEN_METADATA 0x407c0c0a
#endif

#ifndef ELF_NOTE_FDO
# define ELF_NOTE_FDO "FDO"
#endif

extern int verbose;

static int enqueue_elf_dlopen(const char *filename, int fd) __attribute__((__nonnull__ (1)));
static int enqueue_shared_libraries(const char *filename, uint8_t elf_class, uint16_t elf_machine, bool can_cache) __attribute__((__nonnull__ (1)));
static FILE *pipe_command(char *const argv[], pid_t *pid_out);
static int pipe_command_close(FILE *stream, pid_t pid);

static FILE *pipe_command(char *const argv[], pid_t *pid_out)
{
	int pipefd[2];
	pid_t pid;
	FILE *stream;

	if (!pid_out) {
		errno = EINVAL;
		return NULL;
	}

	if (pipe(pipefd) < 0)
		return NULL;

	pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return NULL;
	}

	if (pid == 0) {
		close(pipefd[0]);

		if (dup2(pipefd[1], STDOUT_FILENO) < 0 ||
		    dup2(pipefd[1], STDERR_FILENO) < 0)
			_exit(EXIT_FAILURE);

		if (pipefd[1] > STDERR_FILENO)
			close(pipefd[1]);

		execvp(argv[0], argv);
		_exit(EXIT_FAILURE);
	}

	close(pipefd[1]);

	stream = fdopen(pipefd[0], "r");
	if (!stream) {
		close(pipefd[0]);
		(void) TEMP_FAILURE_RETRY(waitpid(pid, NULL, 0));
		return NULL;
	}

	*pid_out = pid;

	return stream;
}

static int pipe_command_close(FILE *stream, pid_t pid)
{
	int status;
	int ret = 0;

	if (pid <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (fclose(stream) != 0)
		ret = -1;

	if (TEMP_FAILURE_RETRY(waitpid(pid, &status, 0)) < 0)
		ret = -1;

	return ret;
}


struct elf_cache_entry {
	uint8_t elf_class;
	uint16_t elf_machine;
	char *soname;
	char *path;
};

static void *elf_cache_root = NULL;

static int elf_cache_compare(const void *a, const void *b)
{
	const struct elf_cache_entry *ea = a;
	const struct elf_cache_entry *eb = b;

	if (ea->elf_class != eb->elf_class)
		return (int)ea->elf_class - (int)eb->elf_class;
	if (ea->elf_machine != eb->elf_machine)
		return (int)ea->elf_machine - (int)eb->elf_machine;
	return strcmp(ea->soname, eb->soname);
}

static const char *elf_cache_lookup(uint8_t elf_class, uint16_t elf_machine, const char *soname)
{
	struct elf_cache_entry key = {
		.elf_class = elf_class,
		.elf_machine = elf_machine,
		.soname = (char *) soname,
	};
	void *res = tfind(&key, &elf_cache_root, elf_cache_compare);
	if (!res)
		return NULL;
	return (*(struct elf_cache_entry **)res)->path;
}

static void elf_cache_insert(uint8_t elf_class, uint16_t elf_machine, const char *soname, const char *path)
{
	struct elf_cache_entry *entry = xcalloc(1, sizeof(*entry));
	entry->elf_class = elf_class;
	entry->elf_machine = elf_machine;
	entry->soname = xstrdup(soname);
	entry->path = xstrdup(path);

	void *res = tsearch(entry, &elf_cache_root, elf_cache_compare);
	if (!res)
		err(EX_OSERR, "tsearch");
	if (*(struct elf_cache_entry **)res != entry) {
		free(entry->soname);
		free(entry->path);
		free(entry);
	}
}

static void free_elf_cache_entry(void *nodep)
{
	struct elf_cache_entry *e = nodep;
	free(e->soname);
	free(e->path);
	free(e);
}

void elf_cache_destroy(void)
{
#ifdef HAVE_TDESTROY
	tdestroy(elf_cache_root, free_elf_cache_entry);
#endif
	elf_cache_root = NULL;
}

static bool is_elf_cache_disabled(void)
{
	static int disabled = -1;
	if (disabled == -1) {
		const char *env = getenv("IGNORE_PUT_ELF_CACHE");
		disabled = (env && *env && strcmp(env, "0") != 0) ? 1 : 0;
	}
	return disabled == 1;
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
			if (nhdr.n_type == NT_FDO_DLOPEN_METADATA &&
			    nhdr.n_namesz == sizeof(ELF_NOTE_FDO) &&
			    !strcmp(data->d_buf + n_off, ELF_NOTE_FDO)) {
				char *json = xstrndup(data->d_buf + d_off, nhdr.n_descsz);
				library[0] = '\0';

				process_json_metadata(filename, json, library);
				free(json);
			}
			off = next;
		}
	}
end:
	elf_end(e);
err:
	return ret;
}

struct pending_cache {
	char *soname;
	char *path;
};

static int enqueue_shared_libraries(const char *filename, uint8_t elf_class, uint16_t elf_machine, bool can_cache)
{
	int ret = 0;
	pid_t pid = -1;
	FILE *pfd;
	char *line = NULL;
	size_t len = 0;
	ssize_t n;
	char *const argv[] = { (char *) "ldd", (char *) "--", (char *) filename, NULL };
	struct pending_cache *pending = NULL;
	size_t pending_cnt = 0, pending_cap = 0;

	pfd = pipe_command(argv, &pid);
	if (!pfd) {
		warn("pipe_command(ldd): %s", filename);
		return -1;
	}

	while ((n = getline(&line, &len, pfd)) != -1) {
		char *p;
		char *soname = NULL;

		if (line[n - 1] == '\n')
			line[n - 1] = '\0';

		p = strstr(line, "(0x");
		if (!p)
			continue;
		*p = '\0';

		while (p > line && isspace((unsigned char)p[-1])) {
			p--;
			*p = '\0';
		}

		char *sep = strstr(line, " => ");
		if (sep) {
			soname = line;
			while (*soname != '\0' && isspace((unsigned char)*soname))
				soname++;
			char *end_soname = sep;
			while (end_soname > soname && isspace((unsigned char)end_soname[-1]))
				end_soname--;
			*end_soname = '\0';
			p = sep + 4;
		} else {
			p = line;
		}

		while (*p != '\0' && isspace((unsigned char)*p))
			*p++ = '\0';

		if (*p != '/')
			continue;

		if (can_cache) {
			const char *key_soname = (soname && *soname) ? soname : strrchr(p, '/');
			if (key_soname && *key_soname == '/')
				key_soname++;
			if (key_soname && *key_soname) {
				if (pending_cnt == pending_cap) {
					pending_cap = pending_cap ? pending_cap * 2 : 8;
					pending = xrealloc(pending, pending_cap, sizeof(*pending));
				}
				pending[pending_cnt].soname = xstrdup(key_soname);
				pending[pending_cnt].path = xstrdup(p);
				pending_cnt++;
			}
		}

		if (verbose > 1)
			warnx("shared object '%s' depends on '%s'", filename, p);

		if (is_path_added(p))
			continue;

		enqueue_item(p, -1);
	}

	free(line);

	if (pipe_command_close(pfd, pid) < 0) {
		warn("pipe_command_close(ldd): %s", filename);
		ret = -1;
	} else if (can_cache) {
		for (size_t i = 0; i < pending_cnt; i++) {
			elf_cache_insert(elf_class, elf_machine, pending[i].soname, pending[i].path);
		}
	}

	for (size_t i = 0; i < pending_cnt; i++) {
		free(pending[i].soname);
		free(pending[i].path);
	}
	free(pending);

	return ret;
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
	Elf *e;
	GElf_Ehdr ehdr;
	Elf_Scn *scn;
	Elf_Scn *dynamic_scn = NULL;
	GElf_Shdr dynamic_shdr;
	uint8_t elf_class;
	uint16_t elf_machine;
	char interp[PATH_MAX + 1] = { 0 };

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("%s: elf_begin: %s", filename, elf_errmsg(-1));
		return 0;
	}

	switch (elf_kind(e)) {
		case ELF_K_NONE:
		case ELF_K_AR:
		case ELF_K_COFF:
		case ELF_K_ELF:
			break;
		default:
			goto end;
	}

	if (gelf_getehdr(e, &ehdr) != &ehdr)
		goto end;

	elf_class = ehdr.e_ident[EI_CLASS];
	elf_machine = ehdr.e_machine;

	size_t phnum = 0;
	if (elf_getphdrnum(e, &phnum) == 0 && phnum > 0) {
		for (size_t i = 0; i < phnum; i++) {
			GElf_Phdr phdr;
			if (gelf_getphdr(e, (int) i, &phdr) != &phdr)
				continue;
			if (phdr.p_type == PT_INTERP) {
				if (phdr.p_filesz > 0 && phdr.p_filesz < sizeof(interp)) {
					if (TEMP_FAILURE_RETRY(pread(fd, interp, (size_t) phdr.p_filesz, (off_t) phdr.p_offset)) == (ssize_t) phdr.p_filesz) {
						interp[phdr.p_filesz] = '\0';
						char *end = interp + strlen(interp);
						while (end > interp && isspace((unsigned char)end[-1]))
							*--end = '\0';
					}
				}
				break;
			}
		}
	}

	for (scn = NULL; (scn = elf_nextscn(e, scn)) != NULL;) {
		GElf_Shdr shdr;
		if (gelf_getshdr(scn, &shdr) != &shdr)
			continue;
		if (shdr.sh_type == SHT_DYNAMIC) {
			dynamic_scn = scn;
			dynamic_shdr = shdr;
			break;
		}
	}

	if (!dynamic_scn)
		goto end;

	bool has_rpath = false;
	char **needed_names = NULL;
	size_t needed_cnt = 0, needed_cap = 0;

	Elf_Data *data = elf_getdata(dynamic_scn, NULL);
	if (data && dynamic_shdr.sh_entsize > 0) {
		size_t num_dyn = dynamic_shdr.sh_size / dynamic_shdr.sh_entsize;
		for (size_t i = 0; i < num_dyn; i++) {
			GElf_Dyn dyn;
			if (gelf_getdyn(data, (int) i, &dyn) != &dyn)
				break;
			if (dyn.d_tag == DT_NULL)
				break;
			if (dyn.d_tag == DT_RPATH || dyn.d_tag == DT_RUNPATH) {
				has_rpath = true;
			} else if (dyn.d_tag == DT_NEEDED) {
				const char *soname = elf_strptr(e, dynamic_shdr.sh_link, dyn.d_un.d_val);
				if (soname && *soname) {
					if (needed_cnt == needed_cap) {
						needed_cap = needed_cap ? needed_cap * 2 : 8;
						needed_names = xrealloc(needed_names, needed_cap, sizeof(*needed_names));
					}
					needed_names[needed_cnt++] = xstrdup(soname);
				}
			}
		}
	}

	bool can_use_cache = !has_rpath && !is_elf_cache_disabled();
	bool all_in_cache = can_use_cache;

	if (can_use_cache) {
		for (size_t i = 0; i < needed_cnt; i++) {
			if (!elf_cache_lookup(elf_class, elf_machine, needed_names[i])) {
				all_in_cache = false;
				break;
			}
		}
	}

	if (all_in_cache) {
		for (size_t i = 0; i < needed_cnt; i++) {
			const char *path = elf_cache_lookup(elf_class, elf_machine, needed_names[i]);
			if (verbose > 1)
				warnx("shared object '%s' depends on '%s'", filename, path);
			if (!is_path_added(path))
				enqueue_item(path, -1);
		}
		if (interp[0] == '/') {
			if (verbose > 1)
				warnx("shared object '%s' depends on '%s'", filename, interp);
			if (!is_path_added(interp))
				enqueue_item(interp, -1);
		}
	} else {
		ret = enqueue_shared_libraries(filename, elf_class, elf_machine, can_use_cache);
	}

	for (size_t i = 0; i < needed_cnt; i++)
		free(needed_names[i]);
	free(needed_names);

end:
	elf_end(e);

	if (!ret && dynamic_scn)
		ret = enqueue_elf_dlopen(filename, fd);

	return ret;
}
