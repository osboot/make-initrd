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
				for (int key = 0; key < _KEY_TYPE_MAX; key++) {
					if (!strcmp(yytext, rule_key_names[key])) {
						yylval->num = key;
						return KEY;
					}
				}
				warnx("Invalid key `%s'", yytext);
				return ERROR;
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
				for (int op = 0; op < _OP_TYPE_MAX; op++) {
					if (!strcmp(yytext, rule_op_names[op])) {
						yylval->num = op;
						return OPERATION;
					}
				}
				warnx("Invalid operation `%s'", yytext);
				return ERROR;
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
