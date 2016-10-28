%{
#include <ctype.h>
#include <string.h>

#include "grammar.tab.h"
#include "gencode.h"
static int stoi(char *);
static inline int xdtoi(int);

static YY_BUFFER_STATE in_buffer;
%}

N		([0-9]+|(0X|0x)[0-9A-Fa-f]+)

%%
ip		return IP;
udp		return UDP;
port		return PORT;
dst		return DST;
src		return SRC;

and|"&&"	return AND;
or|"||"		return OR;
not		return '!';

[ \r\n\t]		;

{N}			{ yylval.i = stoi((char *)yytext); return NUM; }

.			{ printf("illegal char '%c'\n", *yytext); }
%%
void
lex_init(const char *buf)
{
	in_buffer = yy_scan_string(buf);
}

/*
 * Do any cleanup necessary after parsing.
 */
void
lex_cleanup()
{

	if (in_buffer != NULL)
		yy_delete_buffer(in_buffer);
	in_buffer = NULL;

}

/*
 * Also define a yywrap.  Note that if we're using flex, it will
 * define a macro to map this identifier to pcap_wrap.
 */
int
yywrap()
{
	return 1;
}

/* Hex digit to integer. */
static inline int xdtoi(int c)
{
	if (isdigit(c))
		return c - '0';
	else if (islower(c))
		return c - 'a' + 10;
	else
		return c - 'A' + 10;
}

/*
 * Convert string to integer.  Just like atoi(), but checks for
 * preceding 0x or 0 and uses hex or octal instead of decimal.
 */
static int stoi(char *s)
{
	int base = 10;
	int n = 0;

	if (*s == '0') {
		if (s[1] == 'x' || s[1] == 'X') {
			s += 2;
			base = 16;
		}
		else {
			base = 8;
			s += 1;
		}
	}
	while (*s)
		n = n * base + xdtoi(*s++);

	return n;
}



