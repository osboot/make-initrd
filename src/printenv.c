#include <unistd.h>
#include <stdio.h>

static const char usage[] = "Usage: printenv [-q|-h]\n";
static int quote = 0;

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

int main(int argc, char *argv[], char *envp[])
{
	int i = 0, j;

	if (argc == 2 && argv[1][0] == '-') {
		if (argv[1][1] == 'h') {
			if (write(1, usage, sizeof(usage) - 1) == -1)
				_exit(1);
			return 0;
		}
		if (argv[1][1] == 'q')
			quote = 1;
	}

	while (envp[i]) {
		j = 0;
		while(envp[i][j] != '=') j++;
		envp[i][j] = '\0';

		j = 0;
		while(envp[i][j])
			print_ch(envp[i][j++]);
		j++;

		write_ch('=');
		if (quote)
			write_ch('"');

		while(envp[i][j])
			print_ch(envp[i][j++]);

		if (quote)
			write_ch('"');
		write_ch('\n');
		i++;
	}

	return 0;
}
