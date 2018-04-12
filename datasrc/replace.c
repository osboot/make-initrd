#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

static const char usage[] = "Usage: replace [-s<stop-at-char>] [-r] [-q] <src> <dst> [<string> ...]\n";
static int revert         = 0;
static int quote          = 0;
static char stop_at       = '\0';

static void
write_ch(const char c)
{
	if (write(1, &c, 1) == -1)
		_exit(1);
}

static void
print_ch(const char c)
{
	if (quote && (c == '\\' || c == '"' || c == '$' || c == '`'))
		write_ch('\\');
	write_ch(c);
}

#ifndef SIMPLE
#define err(x)                                                             \
	do {                                                               \
		if (write(2, x, strlen(x)) > 0 && write(2, "\n", 1) > 0) { \
		};                                                         \
		_exit(1);                                                  \
	} while (0)

static char *
add_sequence(char *res, size_t *rlen, unsigned int *i, char *str, size_t len)
{
	size_t j   = 0;
	size_t siz = (*rlen);

	while (siz < (strlen(res) + 1 + len)) {
		siz += BUFSIZ;
		if (!(res = realloc(res, siz)))
			err("realloc failed");
	}

	while (str && j < len)
		res[(*i)++] = str[j++];
	res[(*i)] = '\0';

	(*rlen) = siz;
	return res;
}

static char *
expand_sequence(char *res, size_t *rlen, unsigned int *i, char from, char to)
{
	size_t siz = (*rlen);

	while (siz < (strlen(res) + 1 + (unsigned) (to - from))) {
		siz += BUFSIZ;
		if (!(res = realloc(res, siz)))
			err("realloc failed");
	}

	while (to >= from)
		res[(*i)++] = from++;
	res[(*i)] = '\0';

	(*rlen) = siz;
	return res;
}

static char *
expand(char *src)
{
	unsigned int i = 0, ri = 0, class = 0;
	char prev = '\0';

	char *res   = NULL;
	size_t rlen = BUFSIZ;

	if (!(res = (char *) malloc(rlen)))
		err("malloc failed");
	res[0] = '\0';

	while (src[i]) {
		if (prev == '\\') {
			res  = add_sequence(res, &rlen, &ri, src + i++, 1);
			prev = '\0';
			continue;
		}

		switch (src[i]) {
			case '[':
				if (class) {
					// http://www.regular-expressions.info/posixbrackets.html

					if (!strncasecmp(src + i, "[:xdigit:]", 10)) {
						res = expand_sequence(res, &rlen, &ri, 'A', 'F');
						res = expand_sequence(res, &rlen, &ri, 'a', 'f');
						res = expand_sequence(res, &rlen, &ri, '0', '9');
						i += 10;
					} else if (!strncasecmp(src + i, "[:word:]", 8)) {
						res = expand_sequence(res, &rlen, &ri, 'A', 'Z');
						res = expand_sequence(res, &rlen, &ri, 'a', 'z');
						res = expand_sequence(res, &rlen, &ri, '0', '9');
						res = add_sequence(res, &rlen, &ri, (char *) "_", 1);
						i += 8;
					} else if (!strncasecmp(src + i, "[:alnum:]", 9)) {
						res = expand_sequence(res, &rlen, &ri, 'A', 'Z');
						res = expand_sequence(res, &rlen, &ri, 'a', 'z');
						res = expand_sequence(res, &rlen, &ri, '0', '9');
						i += 9;
					} else if (!strncasecmp(src + i, "[:alpha:]", 9)) {
						res = expand_sequence(res, &rlen, &ri, 'A', 'Z');
						res = expand_sequence(res, &rlen, &ri, 'a', 'z');
						i += 9;
					} else if (!strncasecmp(src + i, "[:digit:]", 9)) {
						res = expand_sequence(res, &rlen, &ri, '0', '9');
						i += 9;
					} else if (!strncasecmp(src + i, "[:upper:]", 9)) {
						res = expand_sequence(res, &rlen, &ri, 'A', 'Z');
						i += 9;
					} else if (!strncasecmp(src + i, "[:lower:]", 9)) {
						res = expand_sequence(res, &rlen, &ri, 'a', 'z');
						i += 9;
					} else if (!strncasecmp(src + i, "[:blank:]", 9)) {
						res = add_sequence(res, &rlen, &ri, (char *) " \t", 2);
						i += 9;
					} else if (!strncasecmp(src + i, "[:space:]", 9)) {
						res = add_sequence(res, &rlen, &ri, (char *) " \t\r\n\v\f", 6);
						i += 9;
					} else {
						err("Unexpected symbol `['\n");
					}
					continue;
				}
				class = 1;
				break;
			case ']':
				if (!class)
					err("Unexpected symbol `]'\n");
				class = 0;
				break;
			case '\\':
				break;
			default:
				if (class && strlen(src + i) >= 3) {
					if (isalnum(src[i]) && isalnum(src[i + 2]) && src[i + 1] == '-') {
						res = expand_sequence(res, &rlen, &ri, src[i], src[i + 2]);
						i += 3;
						continue;
					}
				}
				res = add_sequence(res, &rlen, &ri, src + i, 1);
		}
		prev = src[i++];
	}
	if (class)
		err("Unexpected end of pattern\n");

	res[ri] = '\0';
	return res;
}
#endif

int
main(int argc, char **argv)
{
	size_t i = 1, j, k, len = 0;
	char *src, *dst;

	if (argc < 3) {
		if (write(1, usage, sizeof(usage) - 1) == -1)
			_exit(EXIT_FAILURE);
		return EXIT_SUCCESS;
	}

	while (argv[i] && argv[i][0] == '-') {
		if (!argv[i][1])
			break;
		else if (argv[i][1] == 'q')
			quote = 1;
		else if (argv[i][1] == 'r')
			revert = 1;
		else if (argv[i][1] == 's' && argv[i][2])
			stop_at = argv[i][2];
		i++;
	}

#ifndef SIMPLE
	src = expand(argv[i++]);
	dst = expand(argv[i++]);
#else
	src = argv[i++];
	dst = argv[i++];
#endif
	//	printf("src={%s}\n", src);
	//	printf("dst={%s}\n", dst);

	while (dst[len])
		len++;

	while (argv[i]) {
		if (quote)
			write_ch('"');

		j = 0;
		while (argv[i][j]) {
			if (argv[i][j] == stop_at)
				break;
			k = 0;
			while (src[k] && argv[i][j] != src[k])
				k++;

			if (revert)
				(!src[k])
				    ? print_ch(dst[len - 1])
				    : print_ch(argv[i][j]);
			else
				(src[k])
				    ? print_ch(dst[(k < len) ? k : len - 1])
				    : print_ch(argv[i][j]);
			j++;
		}
		while (argv[i][j])
			print_ch(argv[i][j++]);

		if (quote)
			write_ch('"');
		write_ch('\n');
		i++;
	}
#ifndef SIMPLE
	free(src);
	free(dst);
#endif
	return EXIT_SUCCESS;
}
