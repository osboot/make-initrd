// SPDX-License-Identifier: GPL-3.0-only
#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/prctl.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#include <link.h>
#include <dlfcn.h>

#include <json-c/json.h>
#include <json-c/json_visit.h>

#include "config.h"
#include "memory.h"
#include "elf_dlopen.h"

extern int verbose;

struct elf_metadata {
	const char *feature;
	const char *description;
	const char *priority;
};

struct visit_data {
	int retcode;
	int depth;
	const char *filename;
	char *outbuf;
};

static int in_list(const char *list, const char delim, const char *value)
{
	const char *s, *e, *p;

	if (!list)
		return 0;

	s = p = list;
	e = s + strlen(s);

	while (p < e) {
		p = strchr(s, delim);
		if (!p)
			p = e + 1;
		if (!strncmp(s, value, (size_t)(p - s)))
			return 1;
		s = p + 1;
	}
	return 0;
}

static int in_envvar(const char *envname, const char *value)
{
	return in_list(getenv(envname), ',', value);
}

static pid_t waitpid_retry(pid_t pid, int *wstatus, int options)
{
	return (pid_t) TEMP_FAILURE_RETRY(waitpid(pid, wstatus, options));
}

static int resolve_soname(const char **soname, size_t len, char *outbuf)
{
	int pipefd[2];
	ssize_t n;
	size_t offset, siz;
	pid_t pid;
	void *handle;
	int ret = -1;

	if (pipe(pipefd) < 0) {
		warn("pipe");
		return -1;
	}

	pid = fork();
	if (pid < 0) {
		warn("fork");
		goto out_parent;
	}

	if (pid > 0) {
		offset = 0;
		siz = 0;

		errno = 0;
		n = TEMP_FAILURE_RETRY(read(pipefd[0], &siz, sizeof(siz)));
		if (n < 0) {
			if (errno != EPIPE)
				warn("read(size)");
			goto out_parent;
		}
		if (!siz) {
			ret = 0;
			goto out_parent;
		}

		while (siz > 0) {
			errno = 0;
			n = TEMP_FAILURE_RETRY(read(pipefd[0], outbuf + offset, siz));
			if (n < 0) {
				if (errno != EPIPE)
					warn("read(library)");
				goto out_parent;
			}
			siz -= (size_t) n;
			offset += (size_t) n;
		}
		ret = 0;
out_parent:
		int status;
		if (waitpid_retry(pid, &status, 0) < 0)
			warn("waitpid");

		close(pipefd[0]); // read
		close(pipefd[1]); // write

		return ret;
	}

	if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
		warn("prctl(PR_SET_PDEATHSIG)");

	handle = NULL;
	ret = EXIT_FAILURE;

	for (size_t i = 0; i < len; i++) {
		struct link_map *map = NULL;

		if (handle)
			dlclose(handle);

		offset = 0;
		siz = 0;

		if ((handle = dlopen(soname[i], RTLD_LAZY)) != NULL) {
			if (dlinfo(handle, RTLD_DI_LINKMAP, &map) == 0) {
				siz = strlen(map->l_name) + 1;
			} else {
				warnx("dladdr(%s) failed: %s", soname[i], dlerror());
			}
		}

		errno = 0;
		n = TEMP_FAILURE_RETRY(write(pipefd[1], &siz, sizeof(siz)));
		if (n < 0) {
			if (errno != EPIPE)
				warn("write(size)");
			goto out_child;
		}

		while (siz > 0) {
			errno = 0;
			n = TEMP_FAILURE_RETRY(write(pipefd[1], map->l_name + offset, siz));
			if (n < 0) {
				if (errno != EPIPE)
					warn("write(library)");
				goto out_child;
			}
			siz -= (size_t) n;
			offset += (size_t) n;
		}

		if (map)
			break;
	}
	ret = EXIT_SUCCESS;
out_child:
	if (handle)
		dlclose(handle);

	close(pipefd[1]); // write
	close(pipefd[0]); // read

	exit(ret);
}

static int visit_userfunc(json_object *jso, int flags, json_object *,
                          const char *, size_t *, void *userarg)
{
	struct visit_data *data = userarg;

	if (!(flags ^ JSON_C_VISIT_SECOND)) {
		data->depth--;
		return JSON_C_VISIT_RETURN_CONTINUE;
	}

	if (data->depth == 0) {
		if (!json_object_is_type(jso, json_type_array))
			errx(EXIT_FAILURE, "%s: unexpected object type on depth %d",
			     data->filename,
			     data->depth);
		data->depth++;
		return JSON_C_VISIT_RETURN_CONTINUE;
	}

	if (!json_object_is_type(jso, json_type_object))
		errx(EXIT_FAILURE, "%s: unexpected object type on depth %d",
		     data->filename,
		     data->depth);

	struct json_object *value;
	struct elf_metadata elf_metadata = { 0 };

	if (json_object_object_get_ex(jso, "feature", &value))
		elf_metadata.feature = json_object_get_string(value);

	if (json_object_object_get_ex(jso, "priority", &value))
		elf_metadata.priority = json_object_get_string(value);

	if (json_object_object_get_ex(jso, "description", &value))
		elf_metadata.description = json_object_get_string(value);

	if (json_object_object_get_ex(jso, "soname", &value)) {
		size_t array_len = json_object_array_length(value);
		size_t i;
		const char **soname;

		if (!elf_metadata.feature)
			elf_metadata.feature = "-";

		if (!elf_metadata.priority)
			elf_metadata.priority = "recommended";

		if (in_envvar("IGNORE_PUT_DLOPEN_FEATURE", elf_metadata.feature) ||
		    in_envvar("IGNORE_PUT_DLOPEN_PRIORITY", elf_metadata.priority))
			return JSON_C_VISIT_RETURN_SKIP;

		soname = xcalloc(array_len, sizeof(char *));

		for (i = 0; i < array_len; i++) {
			json_object *child = json_object_array_get_idx(value, i);

			if (!json_object_is_type(child, json_type_string))
				errx(EXIT_FAILURE, "%s: unexpected object type of soname value",
				     data->filename);

			soname[i] = json_object_get_string(child);
		}

		data->retcode = resolve_soname(soname, array_len, data->outbuf);
		free(soname);

		if (verbose > 0) {
			warnx("(elf dlopen): feature=%s: priority=%s: shared object '%s' depends on '%s'",
			      elf_metadata.feature,
			      elf_metadata.priority,
			      data->filename, data->outbuf);
		}
	}

	return JSON_C_VISIT_RETURN_SKIP;
}

int process_json_metadata(const char *filename, const char *json, char *outbuf)
{
	struct json_object *obj = json_tokener_parse(json);
	struct visit_data data = { 0 };

	if (!obj)
		return -1;

	data.filename = filename;
	data.outbuf = outbuf;

	json_c_visit(obj, 0, visit_userfunc, &data);
	json_object_put(obj);

	return data.retcode;
}
