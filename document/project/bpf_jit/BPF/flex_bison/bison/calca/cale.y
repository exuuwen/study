%{
#include <stdio.h>
 
int yylex(void);
void yyerror(char *);
%}
%token INTEGER
%left    '+' '-'
%left    '*' '/'

%%
program:
    program statement 
    |
    ;
statement:
     expr    {printf("=%d", $1);}

expr:INTEGER {$$ = $1;}
|expr '+' expr    {$$ = $1 + $3;}
|expr '-' expr    {$$ = $1 - $3;}
|expr '*' expr    {$$ = $1 * $3;}
|expr '/' expr    {$$ = $1 / $3;}
|'('expr')'    {$$ = $2;}

 
%%
void yyerror(char *s){
        printf("%s\n",s);
        return;
}

