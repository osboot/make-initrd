/* SPDX-License-Identifier: GPL-2.0-or-later */

%{
#include <string.h>
#include <err.h>

#define YY_NO_INPUT
#include "udev-rules.h"
#include "udev-rules-parser.h"

static void update_loc(yyscan_t scanner, char *text);
#define YY_USER_ACTION update_loc(yyscanner, yytext);
%}

%option reentrant noyywrap nounput
%option bison-bridge
%option bison-locations

%x STRING

%%

<INITIAL>[ \t]+		{ /* No action and no return */ }
<INITIAL>#.*\n		{ /* No action and no return */ }
<INITIAL>\\\n		{ /* No action and no return */ }
<INITIAL>\n		{
				return EOL;
			}
<INITIAL>,+		{ return COMMA; }
<INITIAL>(ACTION|ATTR|ATTRS|CONST|DEVPATH|DRIVER|DRIVERS|ENV|GOTO|GROUP|IMPORT|KERNEL|KERNELS|LABEL|MODE|NAME|OPTIONS|OWNER|PROGRAM|RESULT|RUN|SECLABEL|SUBSYSTEM|SUBSYSTEMS|SYMLINK|SYSCTL|TAG|TAGS|TEST) {
				if      (!strcmp(yytext, "ACTION"))     yylval->num = KEY_ACTION;
				else if (!strcmp(yytext, "ATTR"))       yylval->num = KEY_ATTR;
				else if (!strcmp(yytext, "ATTRS"))      yylval->num = KEY_ATTRS;
				else if (!strcmp(yytext, "CONST"))      yylval->num = KEY_CONST;
				else if (!strcmp(yytext, "DEVPATH"))    yylval->num = KEY_DEVPATH;
				else if (!strcmp(yytext, "DRIVER"))     yylval->num = KEY_DRIVER;
				else if (!strcmp(yytext, "DRIVERS"))    yylval->num = KEY_DRIVERS;
				else if (!strcmp(yytext, "ENV"))        yylval->num = KEY_ENV;
				else if (!strcmp(yytext, "GOTO"))       yylval->num = KEY_GOTO;
				else if (!strcmp(yytext, "GROUP"))      yylval->num = KEY_GROUP;
				else if (!strcmp(yytext, "IMPORT"))     yylval->num = KEY_IMPORT;
				else if (!strcmp(yytext, "KERNEL"))     yylval->num = KEY_KERNEL;
				else if (!strcmp(yytext, "KERNELS"))    yylval->num = KEY_KERNELS;
				else if (!strcmp(yytext, "LABEL"))      yylval->num = KEY_LABEL;
				else if (!strcmp(yytext, "MODE"))       yylval->num = KEY_MODE;
				else if (!strcmp(yytext, "NAME"))       yylval->num = KEY_NAME;
				else if (!strcmp(yytext, "OPTIONS"))    yylval->num = KEY_OPTIONS;
				else if (!strcmp(yytext, "OWNER"))      yylval->num = KEY_OWNER;
				else if (!strcmp(yytext, "PROGRAM"))    yylval->num = KEY_PROGRAM;
				else if (!strcmp(yytext, "RESULT"))     yylval->num = KEY_RESULT;
				else if (!strcmp(yytext, "RUN"))        yylval->num = KEY_RUN;
				else if (!strcmp(yytext, "SECLABEL"))   yylval->num = KEY_SECLABEL;
				else if (!strcmp(yytext, "SUBSYSTEM"))  yylval->num = KEY_SUBSYSTEM;
				else if (!strcmp(yytext, "SUBSYSTEMS")) yylval->num = KEY_SUBSYSTEMS;
				else if (!strcmp(yytext, "SYMLINK"))    yylval->num = KEY_SYMLINK;
				else if (!strcmp(yytext, "SYSCTL"))     yylval->num = KEY_SYSCTL;
				else if (!strcmp(yytext, "TAG"))        yylval->num = KEY_TAG;
				else if (!strcmp(yytext, "TAGS"))       yylval->num = KEY_TAGS;
				else if (!strcmp(yytext, "TEST"))       yylval->num = KEY_TEST;
				else {
					warnx("Invalid key `%s'", yytext);
					return ERROR;
				}
				return KEY;
			}
<INITIAL>[{][^}]+[}]	{
				size_t n = strlen(yytext + 1);
				yylval->string = new_string();
				string_append_str(yylval->string, yytext + 1, n - 1);
				return ATTR;
			}
<INITIAL>["]		{
				yylval->string = new_string();
				BEGIN STRING;
			}
<STRING>["]		{
				BEGIN INITIAL;
				return VALUE;
			}
<STRING>\\\"		{ string_append_chr(yylval->string, '"');  }
<STRING>\\\\		{ string_append_chr(yylval->string, '\\'); }
<STRING>\\.		{ string_append_chr(yylval->string, yytext[1]); }
<STRING>.		{ string_append_chr(yylval->string, yytext[0]); }
<INITIAL>[:!=+-]=|=	{
				if      (!strcmp(yytext,  "=")) yylval->num = OP_ASSIGN;
				else if (!strcmp(yytext, ":=")) yylval->num = OP_ASSIGN_FINAL;
				else if (!strcmp(yytext, "+=")) yylval->num = OP_ADD;
				else if (!strcmp(yytext, "-=")) yylval->num = OP_REMOVE;
				else if (!strcmp(yytext, "==")) yylval->num = OP_MATCH;
				else if (!strcmp(yytext, "!=")) yylval->num = OP_NOMATCH;
				else {
					warnx("Invalid operation `%s'", yytext);
					return ERROR;
				}
				return OPERATION;
			}
<INITIAL>.		{ return ERROR; }

%%

void update_loc(yyscan_t scanner, char *text)
{
	YYLTYPE *lloc = yyget_lloc(scanner);

	lloc->first_line   = lloc->last_line;
	lloc->first_column = lloc->last_column;

	for (char *s = text; *s != '\0'; s++) {
		if (*s == '\n') {
			lloc->last_line++;
			lloc->last_column = 1;
		} else {
			lloc->last_column++;
		}
	}
}
