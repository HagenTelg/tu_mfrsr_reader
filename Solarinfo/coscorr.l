/*
 * lexer for "cosine correction" parsing
 *
 * 4/5/96 rs
 * add a "hex constant" token that's like $0e1c
 *
 */

%{

#include "cosdefs.h"		/* DOS file names... */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

long Line_Count=1;

#include "tokens.h"

%}

cr		\r
lf		\n          
crlf		\r\n
white_sp	[ \t]+
comment		#[^\n]*
float		-?(([0-9]+)|([0-9]*\.[0-9]*)([eE][-+]?[0-9]+)?)
integer		-?[0-9]+(\.0*)?
hexdig		[0-9a-fA-F]
hexconst	\${hexdig}+
key		KEY
end		END
sn		SN{integer}
we		WE{integer}

/* any combination of characters between ascii 33 and 126 is a variable */

variable	[\!-\~]+  

%%

{comment}				|
{crlf}					|
{cr}					|
{lf}					{ Line_Count++; }
{white_sp}				{ }
{integer}				{ return(TOK_INT); }
{hexconst}				{ return(TOK_HEX); }
{float}					{ return(TOK_FLOAT); }
{key}					{ return(TOK_KEY); }
{end}					{ return(TOK_END); } 
MULTIFILTER				{ return(TOK_MULTI); }
MULTIFILTER32				{ return(TOK_MULTI32); }
TABLE					{ return(TOK_TABLE); }
TABLENL					{ return(TOK_TABLENL); }
NOLANG					{ return(TOK_NOLANG); }
NONE					{ return(TOK_NONE); }
LICOR					{ return(TOK_LICOR); }
LIRAD					|
SIRAD					{ return(TOK_SIRAD); }
VOIDTBL					{ return(TOK_VOID); }
PHOTOMETER				{ return(TOK_PHOTO); }
RADIOMETER				{ return(TOK_RADIO); }
{sn}					{ return(TOK_SN); }
{we}					{ return(TOK_WE); }
{variable}				{ return(TOK_VAR); } 
.					{ return(TOK_UNK); }

%%

extern long Line_count;

/* {lambdas}				{ return(TOK_LAMBDAS); } */

int yyerror(char *p) { 
	return(1);
}

#ifdef YY_TEST

int main(void)
{
    int parse_Si(FILE *);

    parse_Si(stdin);

    return 0;
}


int parse_Si(FILE *fp)
{
  int tok;
  extern char *yytext;

  yyin = fp;
  Line_count = 1;

  while((tok = yylex()) != TOK_EOF) {
	switch(tok) {
		case TOK_COMM : printf(" comment\n"); break;
		case TOK_CR: printf(" cr\n"); break;
		case TOK_WS: printf(" ws"); break;
		case TOK_LF: printf(" lf\n"); break;
		case TOK_INT: printf(" int %s", yytext); break;
		case TOK_FLOAT : printf(" float %s", yytext); break;
		case TOK_END  : printf(" end"); break;
		case TOK_KEY: printf(" key"); break;
		case TOK_LAMBDAS: printf(" lambdas"); break;
		case TOK_VAR: printf(" var %s", yytext); break;
		case TOK_EOF: printf(" eof\n"); break;
		default : { printf("Lexer error at line %ld\n", Line_count); }
	}
   }
}


#endif	/* YY_TEST */
