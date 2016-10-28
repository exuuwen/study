%{
#include <stdlib.h>
#include "cale.tab.h"
 
void yyerror(char *);
%}
%%
[0-9]+ {yylval = atoi(yytext);
        return INTEGER;}
 
[-+()*] {return *yytext;}
 
[/t] ;
 
. yyerror("invalid characters");
 
%%
int yywrap(void)
{
        return 1;
}

