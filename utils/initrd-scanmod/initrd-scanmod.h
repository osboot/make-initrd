// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _INITRD_SCANMOD_H_
#define _INITRD_SCANMOD_H_

#include <sys/types.h>
#include <regex.h>

#include "compiler.h"
#include "cleanup.h"

extern int verbose;
extern int jobs;

// findmodule-common.c
void *xmalloc(size_t size) _alloc_(1) _returns_nonnull_ _must_check_;
void *xrealloc(void *ptr, size_t nmemb, size_t elem_size) _alloc_(2, 3) _returns_nonnull_ _must_check_;
void *xcalloc(size_t nmemb, size_t size) _alloc_(1, 2) _returns_nonnull_ _must_check_;
int xasprintf(char **ptr, const char *fmt, ...) _printf_(2, 3) _wronly_ptr_(1) _nonnull_(2);
char *xstrdup(const char *s) _nonnull_(1) _returns_nonnull_ _must_check_;

// findmodule-rules.c
#define RULESET_HAS_INFO 1
#define RULESET_HAS_SYMBOLS 2
#define RULESET_HAS_PATHS 4

struct ruleset {
	char *filename;
	int flags;
	struct rule_pair **info;
	struct rule_pair **symbols;
	struct rule_pair **paths;
};

typedef enum {
	RULE_MATCH = 0,
	RULE_NOT_MATCH,
} rule_t;

typedef enum {
	KW_ALIAS = 0,
	KW_AUTHOR,
	KW_DEPENDS,
	KW_DESCRIPTION,
	KW_FILENAME,
	KW_FIRMWARE,
	KW_LICENSE,
	KW_NAME,
	KW_SYMBOL,
	KW_UNKNOWN_KEYWORD,
} keyword_t;

extern const char *const keywords[];

struct rule_pair {
	rule_t type;
	keyword_t keyword;
	regex_t *value;
};

void parse_rules(int n_files, char **files) _access_(read_only, 2, 1);
void free_rules(void);

// findmodule-file.c
struct mapfile {
	int fd;
	size_t size;
	const char *filename;
	char *map;
};

int open_map(const char *filename, struct mapfile *f, int quiet) _rdonly_ptr_(1) _wronly_ptr_(2);
void close_map(struct mapfile *f) _rdwr_ptr_(1);

// findmodule-walk.c
void find_modules(const char *kerneldir);

#endif /* _INITRD_SCANMOD_H_ */
