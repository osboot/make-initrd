#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <math.h>

#include <slang.h>
#include <newt.h>
#include <iniparser/iniparser.h>

#define INTERFACE_DONE 0
#define INTERFACE_LIST 1
#define INTERFACE_EDIT 2

int screen_cols;
int screen_rows;
int timeout = 5;

struct winform {
	int wincols;
	int winrows;
	newtComponent form;
	newtComponent labelHint1;
	newtComponent labelHint2;
	struct newtExitStruct es;
};

struct section {
	char *title;
	char *kernel;
	char *initrd;
	char *append;
};

struct config {
	struct {
		char *title;
	} messages;
	int timeout;
	char *cmdline;
	int nr_sections;
	struct section *sections;
};

struct config *cfg;
struct section *current_boot;

static void
create_winform(struct winform *w)
{
	w->wincols = screen_cols - 2;
	w->winrows = screen_rows - 2;
	newtOpenWindow(1, 1, w->wincols, w->winrows, cfg->messages.title);

	w->form = newtForm(NULL, NULL, 0);
	w->labelHint1 = newtLabel(1, w->winrows - 2, "");
	w->labelHint2 = newtLabel(1, w->winrows - 1, "");

	newtFormAddComponents(w->form, w->labelHint1, w->labelHint2, NULL);
}

static void
run_winform(struct winform *w)
{
	newtFormRun(w->form, &w->es);
}

static void
destroy_winform(struct winform *w)
{
	newtFormDestroy(w->form);
}

static void
form_field(newtComponent form, int top, int width, const char *text, char *oldvalue, const char **newvalue)
{
	newtComponent label, entry;

	label = newtLabel(1, top, text);
	entry = newtEntry(strlen(text) + 2, top, oldvalue, width, newvalue, NEWT_FLAG_SCROLL | NEWT_FLAG_RETURNEXIT);

	newtFormAddComponents(form, label, entry, NULL);
}

static void
iface_edit(int *iface)
{
	const char *initrd, *kernel, *append;
	struct winform w;

	create_winform(&w);

	form_field(w.form, 1, w.wincols - 10, "Kernel:", current_boot->kernel, &kernel);
	form_field(w.form, 2, w.wincols - 10, "Initrd:", current_boot->initrd, &initrd);
	form_field(w.form, 3, w.wincols - 10, "Append:", current_boot->append, &append);
	newtLabelSetText(w.labelHint2, "Press Enter to return to the main menu");

	run_winform(&w);

	free(current_boot->kernel);
	free(current_boot->initrd);
	free(current_boot->append);

	current_boot->kernel = strdup(kernel);
	current_boot->initrd = strdup(initrd);
	current_boot->append = strdup(append);

	*iface = INTERFACE_LIST;

	destroy_winform(&w);
}

static void
iface_list(int *iface)
{
	struct winform w;

	create_winform(&w);

	newtFormAddHotKey(w.form, NEWT_KEY_ESCAPE);
	newtFormAddHotKey(w.form, NEWT_KEY_ENTER);
	newtLabelSetText(w.labelHint2, "Press Enter to continue boot. Press any other key to interupt automatic boot");

	int listboxSize = (int) round(w.winrows / 2);
	newtComponent listbox = newtListbox(1, 0, listboxSize, NEWT_FLAG_SCROLL | NEWT_FLAG_BORDER);

	int current = 0;
	for (int i = 0; i < cfg->nr_sections; i++) {
		if (current_boot == (cfg->sections + i))
			current = i;
		newtListboxAppendEntry(listbox, cfg->sections[i].title, cfg->sections + i);
	}

	newtListboxSetWidth(listbox, w.wincols - 2);
	newtListboxSetCurrent(listbox, current);
	newtFormAddComponent(w.form, listbox);

	newtComponent labelTimeout = newtLabel(1, listboxSize, "");
	newtFormAddComponent(w.form, labelTimeout);

	if (!newtListboxItemCount(listbox)) {
		newtLabelSetText(w.labelHint2, "Press Enter to panic");
		newtLabelSetText(labelTimeout, "No boot entries found");
		run_winform(&w);
		*iface = INTERFACE_DONE;
		goto end;
	}

	if (timeout) {
		int input_pending = 0;

		newtFormSetTimer(w.form, 1);
		run_winform(&w);

		while (timeout && !input_pending) {
			static char counter[1024];
			snprintf(counter, sizeof(counter), "Timeout: %ds", timeout--);
			newtLabelSetText(labelTimeout, counter);
			newtRefresh();
			input_pending = SLang_input_pending(10);
		}

		if (!input_pending || SLang_getkey() == 0x0D) {
			*iface = INTERFACE_DONE;
			goto set;
		}
		timeout = 0;
	}

	newtLabelSetText(labelTimeout, "");
	newtLabelSetText(w.labelHint1, "Press Enter to boot selected entry");
	newtLabelSetText(w.labelHint2, "Press Escape to edit boot entry");

	run_winform(&w);

	if (w.es.reason == NEWT_EXIT_HOTKEY) {
		switch (w.es.u.key) {
			case NEWT_KEY_ESCAPE:
				*iface = INTERFACE_EDIT;
				break;
			case NEWT_KEY_ENTER:
				*iface = INTERFACE_DONE;
				break;
			default:
				break;
		}
	}
set:
	if (newtListboxItemCount(listbox))
		current_boot = newtListboxGetCurrent(listbox);
end:
	destroy_winform(&w);
}

static void
split_section_name(const char *name, char *s, int s_len, char *ss, int ss_len)
{
	int len = (name ? strlen(name) : 0);
	int i, j;

	s[0]  = '\0';
	ss[0] = '\0';

	for (i = len - 1; i >= 0 && isspace(name[i]); i--, len--);
	if (!len)
		return;

	i = 0;
	while (isspace(name[i])) i++;

	for (j = 0; i < len && !isspace(name[i]); i++)
		if (j < s_len)
			s[j++] = name[i];
	s[(j == s_len ? j - 1 : j)] = '\0';

	while (isspace(name[i])) i++;

	if(i == len)
		return;
	len--;

	if (name[i] != '"' || name[len] != '"') {
		error(0, 0, "broken format: `%s'\n", name);
		return;
	}
	i++;

	for (j = 0; i < len && j < ss_len; i++, j++)
		ss[j] = name[i];
	ss[(j == ss_len ? j - 1 : j)] = '\0';
}

static int
parse_config(char *filename)
{
	static char section[1024];
	static char subsection[1024];
	static char key[1024];
	static char empty[] = "";
	char *cmdline, *arg, *append;
	int cmdline_len, append_len;
	int rc = -1;

	cmdline = NULL;
	cmdline_len = 0;

	cfg = malloc(sizeof(struct config));
	if (!cfg) {
		error(0, errno, "malloc");
		goto fail;
	}
	cfg->messages.title = "Make-Initrd Bootloader";
	cfg->nr_sections = 0;
	cfg->sections = NULL;

	dictionary *config = iniparser_load(filename);
	int n_sections = iniparser_getnsec(config);

	for (int i = 0; i < n_sections; i++) {
		char *secname = iniparser_getsecname(config, i);
		split_section_name(secname, section, sizeof(section), subsection, sizeof(subsection));

		if (!strcasecmp(section, "global")) {
			cfg->timeout = iniparser_getint(config, "global:timeout", 120);

			cmdline = iniparser_getstring(config, "global:cmdline", empty);
			cmdline_len = strlen(cmdline);
		} else if (!strcasecmp(section, "messages")) {
			cfg->messages.title = iniparser_getstring(config, "messages:title", empty);

		} else if (!strcasecmp(section, "boot")) {
			cfg->sections = realloc(cfg->sections, sizeof(struct section) * (cfg->nr_sections + 1));
			if (!cfg->sections) {
				error(0, errno, "malloc");
				goto fail;
			}

			cfg->sections[cfg->nr_sections].title = strdup(subsection);

			snprintf(key, sizeof(key), "%s:kernel", secname);
			cfg->sections[cfg->nr_sections].kernel = strdup(iniparser_getstring(config, key, empty));

			snprintf(key, sizeof(key), "%s:initrd", secname);
			cfg->sections[cfg->nr_sections].initrd = strdup(iniparser_getstring(config, key, empty));

			snprintf(key, sizeof(key), "%s:append", secname);
			append = iniparser_getstring(config, key, empty);
			append_len = strlen(append);

			int sz = sizeof(char) * (cmdline_len + 1 + append_len + 1);

			if (!(arg = malloc(sz)))
				error(1, errno, "malloc(%d)", sz);
			arg[0] = '\0';

			if (cmdline_len > 0) {
				strcat(arg, cmdline);
				strcat(arg, " ");
			}
			strcat(arg, append);

			cfg->sections[cfg->nr_sections].append = arg;

			cfg->nr_sections++;
		}
	}

	rc = 0;
fail:
	return rc;
}

static void
suspend(void *d __attribute__((unused))) {
	newtSuspend();
	raise(SIGTSTP);
	newtResume();
}

static void
output(const char *filename, const char *value)
{
	FILE *fd;

	if (!(fd = fopen(filename, "w")))
		error(1, errno, "fopen: %s", filename);
	fputs(value, fd);
	fputs("\n", fd);
	fclose(fd);
}

int
main(int argc, char **argv)
{
	if (argc == 1) {
		fprintf(stderr, "Config file required\n");
		return 1;
	}

	if (parse_config(argv[1]) < 0)
		return 1;

	timeout = cfg->timeout;

	newtInit();
	newtCls();

	newtSetSuspendCallback(suspend, NULL);

	newtGetScreenSize(&screen_cols, &screen_rows);
	newtDrawRootText(1, 1, "Make-Initrd Bootloader");

	int iface = INTERFACE_LIST;

	while (iface != INTERFACE_DONE) {
		switch (iface) {
			case INTERFACE_LIST:
				iface_list(&iface);
				break;
			case INTERFACE_EDIT:
				iface_edit(&iface);
				break;
		}
	}

	newtFinished();

	if (current_boot) {
		if (chdir((argc == 3) ? argv[2] : "/tmp") < 0)
			error(1, errno, "chdir");

		output("title", current_boot->title);
		output("kernel", current_boot->kernel);
		output("initrd", current_boot->initrd);
		output("append", current_boot->append);
	}

	return 0;
}

