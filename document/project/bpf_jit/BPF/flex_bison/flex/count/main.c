#include <stdio.h>
extern int num_lines;
extern int num_chars;
extern int num_words;
extern yyin;
extern yyout;
int main(int argc, char **argv)
{
	yyin = fopen(stdin, "r");/*end with ctrl+d*/
	//yyout = fopen(argv[1], "w");
	lexscan();//default is yylex() 
	printf("new # of lines = %d, # of chars = %d, # num_words = %d\n", num_lines, num_chars, num_words);
	return 0;
}
