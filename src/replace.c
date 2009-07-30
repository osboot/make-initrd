//#define SIMPLE 1

#include <unistd.h>
#ifndef SIMPLE
#include <stdlib.h>
#endif

static const char usage[] = "Usage: replace [-s<char>] [-q] <src> <dst> [<string> ...]\n";
static int quote = 0;
static char stop_at = '\0';

static void
write_ch(const char c) {
	if (write(1, &c, 1) == -1)
		_exit(1);
}

static void
print_ch(const char c) {
	if (quote && (c == '\\' || c == '"' || c == '$' || c == '`'))
		write_ch('\\');
	write_ch(c);
}

#ifndef SIMPLE
static void
write_err(const char *s) {
	if (write(2, s, sizeof(&s)-1) == -1)
		_exit(1);
}

#define err(x) { write_err(x); _exit(1); }
static char *
expand(char *src) {
	int i = 0, class = 0, ri = 0;
	char prev = '\0';
	char sequence[2] = { '\0', '\0' };
	char *res = NULL;

	res = (char *) malloc (sizeof(&src)-1);
	if (!res) err("malloc failed");

	while (src[i]) {
		if (prev != '\\') {
			switch (src[i]) {
				case '[':
					if (class)
						err("Unexpected symbol `['\n");
					class = 1;
					goto end2;
				case ']':
					if (!class)
						err("Unexpected symbol `]'\n");
					class = 0;
					goto end2;
				case '-':
					if (class && src[i+1] != '\0') {
						if (sequence[1] == prev)
							err("Unexpected symbol `-'\n");

						sequence[0] = prev;
						sequence[1] = src[i+1];
						goto end2;
					}
			}
		}

		if (src[i] == '\\') {
			if (prev == '\\') {
				res[ri++] = '\\';
				goto end2; 
			}
			goto end1;
		}

		if (src[i] == sequence[1]) {
			res = realloc(res, sizeof(&res)+(unsigned)(sequence[1]-sequence[0]));
			if (!res) err("realloc failed");

			while (sequence[1] >= sequence[0])
				res[ri++] = sequence[0]++;
			goto end1;
		}

		if (!class)
			res[ri++] = src[i];

end1:		prev = src[i++];
		continue;

end2:		prev = '\0';
		i++;
	}
	if (class)
		err("Unexpected end of pattern\n");

	res[ri] = '\0';
	return res;
}
#endif

int
main(int argc, char **argv) {
	size_t i = 1, j, k, len = 0;
	char *src, *dst;

	if (argc < 3) {
		if (write(1, usage, sizeof(usage) - 1) == -1)
			_exit(1);
		return 0;
	}

	while (argv[i] && argv[i][0] == '-') {
		if (!argv[i][1])
			break;
		else if (argv[i][1] == 'q')
			quote = 1;
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
	while (dst[len++]);

	while (argv[i]) {
		if (quote) write_ch('"');

		j = 0;
		while (argv[i][j]) {
			if (argv[i][j] == stop_at)
				break;
			k = 0;
			while (src[k] && argv[i][j] != src[k]) k++;

			if (src[k]) {
				if (k < len) print_ch(dst[k]);
			}
			else
				print_ch(argv[i][j]);
			j++;
		}
		while (argv[i][j])
			print_ch(argv[i][j++]);
			
		if (quote) write_ch('"');
		write_ch('\n');
		i++;
	}
#ifndef SIMPLE
	free(src);
	free(dst);
#endif
	return 0;
}
