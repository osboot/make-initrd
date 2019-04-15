#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <fnmatch.h>
#include <fcntl.h>

#ifndef _GNU_SOURCE
extern char **environ;
#endif

static char *prefix = (char *) "";
static char *quote = (char *) "";
static unsigned int shell = 0;

static void __attribute__ ((noreturn))
show_usage(int ret)
{
	fprintf(stderr, "Usage: environ [-c|-q|-s|-h] [-p PREFIX] [-i include0,include1,...] [-u unset0,unset1,...] [-f filename] [command [arg]...]\n");
	exit(ret);
}

static void
show_environ(char **env)
{
	while (env && *env) {
		char *c, *eq = NULL;

		eq = *env;
		while (*eq != '=') eq++;

		if (shell) {
			for (c = *env; c != eq; c++) {
				if (*c != '_' && !isalnum(*c))
					goto next;
			}
		}

		fprintf(stdout, "%s", prefix);
		fwrite(*env, sizeof(char), (size_t) (eq - *env), stdout);
		fprintf(stdout, "=%s", quote);
		eq++;

		if (*quote != '\0') {
			c = eq;
			while (*c != '\0') {
				char *d = strpbrk(c, "$`\"\\");
				if (!d) {
					fprintf(stdout, "%s", c);
					break;
				}
				fwrite(c, sizeof(char), (size_t) (d - c), stdout);
				fprintf(stdout, "\\%c", *d);
				c = d + 1;
			}
		} else {
			fprintf(stdout, "%s", eq);
		}
		fprintf(stdout, "%s\n", quote);
next:
		env++;
	}
}

static void
set_environ(const char *fname)
{
	char *a, *map = NULL;
	int fd = -1;
	struct stat sb;

	if ((fd = open(fname, O_RDONLY | O_CLOEXEC)) < 0) {
		fprintf(stderr, "ERROR: open: %s: %m\n", fname);
		exit(1);
	}

	if (fstat(fd, &sb) < 0) {
		fprintf(stderr, "ERROR: fstat: %s: %m\n", fname);
		exit(1);
	}

	if (!sb.st_size) {
		close(fd);
		return;
	}

	if ((a = map = mmap(NULL, (size_t) sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "ERROR: mmap: %s: %m\n", fname);
		exit(1);
	}

	while (a && *a) {
		char *name, *value;
		size_t namesz, valuesz;
		FILE *valfd;
		int is_quote = 0;

		while (isspace(*a)) a++;

		if (*a == '\0')
			break;

		namesz = valuesz = 0;

		while (a[namesz] != '=') {
			if (a[namesz] == '\0') {
				fprintf(stderr, "ERROR: open: %s: expect '=' character for '%s', but found EOF\n", fname, a);
				exit(1);
			}
			namesz++;
		}

		name = strndup(a, namesz);
		a += namesz + 1;

		if (*a != '"') {
			fprintf(stderr, "ERROR: open: %s: expect opening quote not found for '%s', but '%c' found\n", fname, name, *a);
			exit(1);
		}

		a++;

		if ((valfd = open_memstream(&value, &valuesz)) == NULL) {
			fprintf(stderr, "ERROR: open_memstream: %m\n");
			exit(1);
		}

		while (*a != '\0') {
			if (!is_quote) {
				if (*a == '"')
					break;

				if (*a == '\\') {
					is_quote = 1;
					a++;
					continue;
				}
			} else {
				is_quote = 0;
			}

			fprintf(valfd, "%c", *a);
			a++;
		}

		fclose(valfd);

		if (*a != '"') {
			fprintf(stderr, "ERROR: open: %s: expect closing quote not found for '%s', but '%c' found\n", fname, name, *a);
			exit(1);
		}

		a++;

		setenv(name, value, 1);

		free(name);
		free(value);
	}

	munmap(map, (size_t) sb.st_size);
	close(fd);
}

static void
unset_environ(char **envp, char *patterns, int invert_match)
{
	size_t i, j, max = 0, envp_sz = 0;
	char *pattern, *next, *s, **e, **last, *env;
	ssize_t *envname;

	if (!patterns)
		return;

	last = envp;
	while (last && *last) {
		envp_sz++;
		last++;
	}

	envname = calloc(envp_sz, sizeof(ssize_t));

	if (!envname) {
		fprintf(stderr, "ERROR: calloc: %m\n");
		exit(1);
	}

	e = last;
	i = envp_sz;

	while (envp <= e) {
		if (!*e)
			goto next;

		j = 0;
		while ((*e)[j] != '=' && (*e)[j] != '\0')
			j++;

		if ((*e)[j] == '\0')
			goto next;

		if (j > max)
			max = j;

		envname[i] = (ssize_t) j;
next:
		e--;
		i--;
	}

	env = malloc(sizeof(char) * (max + 1));

	if (!env) {
		fprintf(stderr, "ERROR: malloc: %m\n");
		exit(1);
	}

	pattern = s = patterns;
	next = NULL;

	while (pattern) {
		for (; s[0] != ',' && s[0] != '\0'; s++);

		next = (*s == ',') ? s + 1 : NULL;

		s[0] = '\0';
		s++;

		e = last;
		i = envp_sz;

		while (envp <= e) {
			if (*e && envname[i] > 0) {
				j = (size_t) envname[i];

				strncpy(env, *e, j);
				env[j] = '\0';

				if (!fnmatch(pattern, env, 0))
					envname[i] *= -1;
			}

			e--;
			i--;
		}

		pattern = next;
	}

	e = last;
	i = envp_sz;

	while (envp <= e) {
		if (*e) {
			j = 0;

			if (!invert_match) {
				if (envname[i] < 0)
					j = (size_t) (envname[i] * -1);
			} else if (envname[i] > 0) {
				j = (size_t) envname[i];
			}

			if (j > 0) {
				strncpy(env, *e, j);
				env[j] = '\0';
				unsetenv(env);
			}
		}

		e--;
		i--;
	}

	free(envname);
	free(env);
}

int
main(int argc, char *argv[])
{
	int o;

	while ((o = getopt(argc, argv, "cf:hp:qsi:u:")) != -1) {
		switch (o) {
			case 'c':
				clearenv();
				break;
			case 'f':
				set_environ(optarg);
				break;
			case 'p':
				prefix = optarg;
				break;
			case 'q':
				quote = (char *) "\"";
				break;
			case 's':
				shell = 1;
				break;
			case 'i':
				unset_environ(environ, optarg, 1);
				break;
			case 'u':
				unset_environ(environ, optarg, 0);
				break;
			case 'h':
				show_usage(0);
			case '?':
				show_usage(1);
		}
	}

	if (optind < argc) {
		execvp(argv[optind], argv + optind);
		fprintf(stderr, "ERROR: execvp: %s: %m\n", argv[optind]);
		return 1;
	}

	show_environ(environ);

	return 0;
}
