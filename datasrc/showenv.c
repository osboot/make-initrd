#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

static const char usage[] = "Usage: showenv [-q|-s|-h]\n";
static unsigned int quote = 0;
static unsigned int shell = 0;

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

int main(int argc, char *argv[], char *envp[])
{
	unsigned int i = 0, j;

	if (argc == 2 && argv[1][0] == '-') {
		if (argv[1][1] == 'h') {
			if (write(1, usage, sizeof(usage) - 1) == -1)
				_exit(1);
			return 0;
		} else if (argv[1][1] == 'q') {
			quote = 1;
		} else if (argv[1][1] == 's') {
			shell = 1;
		}
	}

	while (envp[i]) {
		j = 0;
		while(envp[i][j] != '=') j++;
		envp[i][j] = '\0';

		if (shell) {
			unsigned int k = 0;

			while(envp[i][k] && (isalnum(envp[i][k]) || envp[i][k] == '_')) k++;

			if (j != k) {
				i++;
				continue;
			}
		}
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
