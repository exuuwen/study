%{
#include <stdlib.h>
#include <stdio.h>

#include "gencode.h"


#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)
int n_errors = 0;
static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };
static void
yyerror(const char *msg)
{
	++n_errors;
	 printf("%s\n",msg);
}

int pcap_parse()
{
	return (yyparse());
}

%}


%union {
	int i;
	bpf_u_int32 h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		int atmfieldtype;
		int mtp3fieldtype;
		struct block *b;
	} blk;
	struct block *rblk;
}


%type	<blk>	expr id  term  rterm
%type	<blk>	head
%type	<blk>	and or not null prog
%type	<i>	pqual dqual aqual
%type	<i>	pname pnum

%token  PORT 
%token  IP  UDP  TCP
%token  NUM 
%token  DST SRC

%type	<i> NUM

%left  OR AND
%nonassoc UMINUS


%%
prog:	  null expr
{
	printf("prog:null expr\n");
	finish_parse($2.b);
}
	| null
	;
null:	  /* null */		{ printf("null\n");$$.q = qerr;}
	;
expr:	  term			{printf("expr:term\n");}
	| expr and term		{ printf("expr:expr and term\n"); gen_and($1.b, $3.b); $$ = $3;}
	| expr or term		{ gen_or($1.b, $3.b); $$ = $3; }	
	;
	
and:	  AND			{ printf("and:AND\n");$$ = $<blk>0; }
	;

or:	  OR			{ $$ = $<blk>0; }
	;

not:	  '!'			{ $$ = $<blk>0; }
	;

term:	  rterm                 { printf("term:rterm\n");}
	| not term		{ gen_not($2.b); $$ = $2; }
	;

head:	  pqual dqual aqual	{ QSET($$.q, $1, $2, $3); }
	| pqual dqual		{ QSET($$.q, $1, $2, Q_DEFAULT); }
	| pqual aqual		{ printf("head:pqual aqual\n");QSET($$.q, $1, Q_DEFAULT, $2); }
	;
rterm:	  head id		{ printf("rterm:head id\n");$$ = $2; }
	| pname			{ printf("rterm:pname\n"); $$.b = gen_proto_abbrev($1); $$.q = qerr; }
	;
/* protocol level qualifiers */
pqual:	  pname			{ printf("pqual:pname\n");}
	|			{ printf("pqual:null\n");$$ = Q_DEFAULT; }
	;
/* 'direction' qualifiers */
dqual:	  SRC			{ $$ = Q_SRC; }
	| DST			{ $$ = Q_DST; }
	| SRC OR DST		{ $$ = Q_OR; }
	| DST OR SRC		{ $$ = Q_OR; }
	| SRC AND DST		{ $$ = Q_AND; }
	| DST AND SRC		{ $$ = Q_AND; }
	
	;
/* address type qualifiers */
aqual:	  PORT			{ printf("aqual:port\n");$$ = Q_PORT; }
	;
pname:	  IP			{ printf("pname:IP\n");$$ = Q_IP; }
	| TCP			{ $$ = Q_TCP; }
	| UDP			{ printf("pname:UDP\n");$$ = Q_UDP; }
	;

id:	  pnum			{ printf("id:pnum\n");$$.b = gen_ncode(NULL, (bpf_u_int32)$1, $$.q = $<blk>0.q); }
	;
pnum:	  NUM			{printf("pnum:NUM\n");}
	;
%%
