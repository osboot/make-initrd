#ifndef _INITRD_SCANMOD_H_
#define _INITRD_SCANMOD_H_

#include <sys/types.h>
#include <regex.h>

extern int verbose;

// findmodule-common.c
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t nmemb, size_t elem_size);
void *xcalloc(size_t nmemb, size_t size);
void *xfree(void *ptr);
int xasprintf(char **ptr, const char *fmt, ...);
char *xstrdup(const char *s);

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

static const char *const keywords[] = {
	[KW_ALIAS]       = "alias",
	[KW_AUTHOR]      = "author",
	[KW_DEPENDS]     = "depends",
	[KW_DESCRIPTION] = "description",
	[KW_FILENAME]    = "filename",
	[KW_FIRMWARE]    = "firmware",
	[KW_LICENSE]     = "license",
	[KW_NAME]        = "name",
	[KW_SYMBOL]      = "symbol",
	NULL
};

struct rule_pair {
	rule_t type;
	keyword_t keyword;
	regex_t *value;
};

void parse_rules(int n_files, char **files);
void free_rules(void);

// findmodule-file.c
struct mapfile {
	int fd;
	size_t size;
	const char *filename;
	char *map;
};

int open_map(const char *filename, struct mapfile *f, int quiet);
void close_map(struct mapfile *f);

// findmodule-walk.c
void find_modules(const char *kerneldir);

#endif /* _INITRD_SCANMOD_H_ */
