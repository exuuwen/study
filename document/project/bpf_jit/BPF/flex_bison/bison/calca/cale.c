#include <stdio.h>
extern yyin;
extern yyout;
int main(int argc, char **argv)
{
	yyin = fopen(stdin, "r");
	
	//yylex();
	yyparse();
	return 0;
}
