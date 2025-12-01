
/*  A Bison parser, made from coscorr.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_COMM	258
#define	TOK_CR	259
#define	TOK_WS	260
#define	TOK_LF	261
#define	TOK_INT	262
#define	TOK_FLOAT	263
#define	TOK_END	264
#define	TOK_KEY	265
#define	TOK_LAMBDAS	266
#define	TOK_VAR	267
#define	TOK_EOF	268
#define	TOK_CRLF	269
#define	TOK_UNK	270
#define	TOK_SN	271
#define	TOK_WE	272
#define	TOK_MULTI	273
#define	TOK_NONE	274
#define	TOK_TABLE	275
#define	TOK_TABLENL	276
#define	TOK_NOLANG	277
#define	TOK_LICOR	278
#define	TOK_SIRAD	279
#define	TOK_VOID	280
#define	TOK_PHOTO	281
#define	TOK_RADIO	282
#define	TOK_MULTI32	283
#define	TOK_HEX	284

#line 9 "coscorr.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** 10/27/95 rs
** couple things in here:
** corr_kind not initialized in "new_channel"; fixed, and
** removed restriction on exactly 1 key channel; can now have 0 or 1.
**
** 4/5/96 rs
** the ids in the header can be hex or decimal.
**
*/

#include "cosdefs.h"

extern solarinfo	*si;
extern char		*yytext;

extern long		Line_Count;

static int		ret, hed_ind = 0, lam_ind = 0, sn_i, we_i,
			sn_ind, we_ind,		/* current table */
			*table_status=NULL;
			/* 0=none; 1=needs; 2=found we; 4=found sn */
static double		hed[16];
static char		err[256];

int			setup_header(void),
			setup_lambdas(void),
			new_channel(void),
			setup_sn_table(void),
			setup_we_table(void),
			new_sn_number(void),
			new_we_number(void),
			finish_tables(void);

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		63
#define	YYFLAG		-32768
#define	YYNTBASE	30

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 56)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     2,     8,    11,    13,    15,    17,    19,    21,
    23,    26,    28,    30,    32,    33,    38,    40,    41,    43,
    46,    47,    52,    53,    54,    60,    62,    65,    67,    69,
    71,    73,    75,    77,    79,    81,    83,    86,    88,    90,
    92,    93,    98,    99,   104,   106,   109,   111
};

static const short yyrhs[] = {    -1,
     0,    33,    31,    37,    32,    48,     0,    34,    35,     0,
    18,     0,    26,     0,    27,     0,    28,     0,    12,     0,
    36,     0,    35,    36,     0,     7,     0,     8,     0,    29,
     0,     0,    12,    38,    40,    39,     0,     9,     0,     0,
    41,     0,    40,    41,     0,     0,    12,    42,    43,    46,
     0,     0,     0,     7,    44,     7,    45,     7,     0,    47,
     0,    46,    47,     0,    19,     0,    22,     0,    23,     0,
    24,     0,    20,     0,    21,     0,    25,     0,    10,     0,
    49,     0,    48,    49,     0,    50,     0,    52,     0,     1,
     0,     0,    16,    51,    54,    39,     0,     0,    17,    53,
    55,    39,     0,    36,     0,    54,    36,     0,    36,     0,
    55,    36,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    51,    52,    53,    56,    59,    59,    60,    61,    62,    71,
    71,    75,    75,    76,    80,    87,    90,    90,   100,   100,
   104,   105,   110,   111,   112,   115,   115,   119,   119,   120,
   121,   122,   126,   130,   134,   138,   138,   142,   142,   143,
   152,   153,   159,   160,   166,   166,   170,   170
};

static const char * const yytname[] = {   "$","error","$undefined.","TOK_COMM",
"TOK_CR","TOK_WS","TOK_LF","TOK_INT","TOK_FLOAT","TOK_END","TOK_KEY","TOK_LAMBDAS",
"TOK_VAR","TOK_EOF","TOK_CRLF","TOK_UNK","TOK_SN","TOK_WE","TOK_MULTI","TOK_NONE",
"TOK_TABLE","TOK_TABLENL","TOK_NOLANG","TOK_LICOR","TOK_SIRAD","TOK_VOID","TOK_PHOTO",
"TOK_RADIO","TOK_MULTI32","TOK_HEX","solarinfo","@1","@2","header","rsr_type",
"header_info","number","lambdas","@3","ending","channels","channel","@4","cols3",
"@5","@6","cor_types","cor_type","tables","table","table_sn","@7","table_we",
"@8","sn_numbers","we_numbers",""
};
#endif

static const short yyr1[] = {     0,
    31,    32,    30,    33,    34,    34,    34,    34,    34,    35,
    35,    36,    36,    36,    38,    37,    39,    39,    40,    40,
    42,    41,    44,    45,    43,    46,    46,    47,    47,    47,
    47,    47,    47,    47,    47,    48,    48,    49,    49,    49,
    51,    50,    53,    52,    54,    54,    55,    55
};

static const short yyr2[] = {     0,
     0,     0,     5,     2,     1,     1,     1,     1,     1,     1,
     2,     1,     1,     1,     0,     4,     1,     0,     1,     2,
     0,     4,     0,     0,     5,     1,     2,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
     0,     4,     0,     4,     1,     2,     1,     2
};

static const short yydefact[] = {     0,
     9,     5,     6,     7,     8,     1,     0,     0,    12,    13,
    14,     4,    10,    15,     2,    11,     0,     0,    21,    18,
    19,    40,    41,    43,     0,    36,    38,    39,     0,    17,
    16,    20,     0,     0,    37,    23,     0,    45,    18,    47,
    18,     0,    35,    28,    32,    33,    29,    30,    31,    34,
    22,    26,    46,    42,    48,    44,    24,    27,     0,    25,
     0,     0,     0
};

static const short yydefgoto[] = {    61,
     8,    18,     6,     7,    12,    13,    15,    17,    31,    20,
    21,    29,    37,    42,    59,    51,    52,    25,    26,    27,
    33,    28,    34,    39,    41
};

static const short yypact[] = {    16,
-32768,-32768,-32768,-32768,-32768,-32768,    -3,     0,-32768,-32768,
-32768,    -3,-32768,-32768,-32768,-32768,     2,    14,-32768,    -2,
-32768,-32768,-32768,-32768,     8,-32768,-32768,-32768,    -1,-32768,
-32768,-32768,    -3,    -3,-32768,-32768,    26,-32768,    -6,-32768,
    -6,     9,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    26,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    10,-32768,
    18,    19,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,   -12,-32768,-32768,   -28,-32768,
    12,-32768,-32768,-32768,-32768,-32768,   -31,-32768,    13,-32768,
-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		51


static const short yytable[] = {    16,
     9,    10,    30,     9,    10,    36,    30,    -3,    22,    19,
    54,    14,    56,    19,    22,    57,    60,    62,    63,    58,
    38,    40,    11,    23,    24,    11,    53,     1,    55,    23,
    24,    32,     0,     2,     0,    43,     0,    35,     0,     0,
     0,     3,     4,     5,    44,    45,    46,    47,    48,    49,
    50
};

static const short yycheck[] = {    12,
     7,     8,     9,     7,     8,     7,     9,     0,     1,    12,
    39,    12,    41,    12,     1,     7,     7,     0,     0,    51,
    33,    34,    29,    16,    17,    29,    39,    12,    41,    16,
    17,    20,    -1,    18,    -1,    10,    -1,    25,    -1,    -1,
    -1,    26,    27,    28,    19,    20,    21,    22,    23,    24,
    25
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/gnu/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "/usr/gnu/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 51 "coscorr.y"
{ if ((ret = setup_header()) != 0) return ret; ;
    break;}
case 2:
#line 52 "coscorr.y"
{ if ((ret = setup_lambdas()) != 0) return ret; ;
    break;}
case 3:
#line 53 "coscorr.y"
{ if ((ret = finish_tables()) != 0) return ret; ;
    break;}
case 5:
#line 59 "coscorr.y"
{ si->type = Multifilter; ;
    break;}
case 6:
#line 60 "coscorr.y"
{ si->type = Photometer; ;
    break;}
case 7:
#line 61 "coscorr.y"
{ si->type = Radiometer; ;
    break;}
case 8:
#line 62 "coscorr.y"
{ si->type = Multifilter32; ;
    break;}
case 9:
#line 63 "coscorr.y"
{
			sprintf(err,
			  "line %ld: unrecognizeable RSR type: '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27;
		;
    break;}
case 10:
#line 71 "coscorr.y"
{ hed[hed_ind++] = my_strtod(yytext, NULL); ;
    break;}
case 11:
#line 72 "coscorr.y"
{ hed[hed_ind++] = my_strtod(yytext, NULL); ;
    break;}
case 15:
#line 80 "coscorr.y"
{ if (!check_var("lambdas")) {
			sprintf(err,
			  "line %ld: expecting LAMBDAS, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		};
    break;}
case 18:
#line 92 "coscorr.y"
{ 
			sprintf(err,
			  "line %ld: expecting END, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		;
    break;}
case 21:
#line 104 "coscorr.y"
{ if ((ret = new_channel()) != 0) return ret; ;
    break;}
case 22:
#line 105 "coscorr.y"
{
		    lam_ind++;
		;
    break;}
case 23:
#line 110 "coscorr.y"
{ si->lambdas[lam_ind].glo_ind = atoi(yytext); ;
    break;}
case 24:
#line 111 "coscorr.y"
{ si->lambdas[lam_ind].diff_ind = atoi(yytext); ;
    break;}
case 25:
#line 112 "coscorr.y"
{ si->lambdas[lam_ind].dir_ind = atoi(yytext); ;
    break;}
case 28:
#line 119 "coscorr.y"
{ si->lambdas[lam_ind].corr_kind += None; ;
    break;}
case 29:
#line 120 "coscorr.y"
{ si->lambdas[lam_ind].corr_kind += Nolang; ;
    break;}
case 30:
#line 121 "coscorr.y"
{ si->lambdas[lam_ind].corr_kind += Licor; ;
    break;}
case 31:
#line 122 "coscorr.y"
{ si->lambdas[lam_ind].corr_kind += Sirad; ;
    break;}
case 32:
#line 123 "coscorr.y"
{
		    si->lambdas[lam_ind].corr_kind += Table;
		    table_status[lam_ind] = 1;	/* needs a table */
		;
    break;}
case 33:
#line 127 "coscorr.y"
{
		    si->lambdas[lam_ind].corr_kind += Tablenl;
		    table_status[lam_ind] = 1;	/* needs a table */
		;
    break;}
case 34:
#line 131 "coscorr.y"
{
		    si->lambdas[lam_ind].corr_kind += Void;
		    table_status[lam_ind] = 1;	/* needs a table */
		;
    break;}
case 35:
#line 135 "coscorr.y"
{ si->lambdas[lam_ind].corr_kind += Key; ;
    break;}
case 40:
#line 144 "coscorr.y"
{ 
			sprintf(err,
			  "line %ld: expecting table label, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		;
    break;}
case 41:
#line 152 "coscorr.y"
{ if ((ret = setup_sn_table()) != 0) return ret; ;
    break;}
case 42:
#line 153 "coscorr.y"
{
		    if (sn_i != 181)
			return 18;	/* not 181 entries in sn table */
		;
    break;}
case 43:
#line 159 "coscorr.y"
{ if ((ret = setup_we_table()) != 0) return ret; ;
    break;}
case 44:
#line 160 "coscorr.y"
{
		    if (we_i != 181)
			return 19;	/* not 181 entries in we table */
		;
    break;}
case 45:
#line 166 "coscorr.y"
{ if ((ret = new_sn_number()) != 0) return ret; ;
    break;}
case 46:
#line 167 "coscorr.y"
{ if ((ret=new_sn_number())!=0) return ret; ;
    break;}
case 47:
#line 170 "coscorr.y"
{ if ((ret = new_we_number()) != 0) return ret; ;
    break;}
case 48:
#line 171 "coscorr.y"
{ if ((ret=new_we_number())!=0) return ret; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "/usr/gnu/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 175 "coscorr.y"


int setup_header(void)
{
    if (si->type == Multifilter32) {
	if (hed_ind != 5 && hed_ind != 6)
	    return 2;	/* header param count wrong */
	si->num_filt = hed[0];
	si->num_chan = hed[1];
	si->unit_id = hed[2];
	si->head_id = hed[3];
	si->valid_from = hed[4];
	si->valid_to = hed_ind == 6 ? hed[5] : si->valid_from - 1.0;
    } else {
	if (hed_ind != 4 && hed_ind != 5)
	    return 3;	/* header param count wrong */
	si->num_filt = hed[0];
	si->num_chan = hed[1];
	si->unit_id = hed[2];
	si->valid_from = hed[3];
	si->valid_to = hed_ind == 5 ? hed[4] : si->valid_from - 1.0;
    }

    return 0;
}



int setup_lambdas(void)
{
    int		i;

    si->num_lambdas = lam_ind;

    /* locate the 'key channel' */
    si->key_ind = -1;
    for (i=0; i<lam_ind; i++) {
	if (si->lambdas[i].corr_kind & Key) {
	    if (si->key_ind != -1)
		return 4;	/* multiple key defs */
	    si->key_ind = i;
	}
    }

/*
** 10/27/95
** Jim says that a key channel is not a necessity...
**
*/
    /*
    if (si->key_ind == -1)
	return 7; */	/* no key def */

    return 0;
}



int new_channel(void)
{
    if (lam_ind == 0) {
	if ((si->lambdas = malloc(sizeof(lambda))) == NULL)
	    return 5;		/* alloc failed lam 0 */
	if ((table_status = malloc(sizeof(int))) == NULL)
	    return 5;
    } else {
	if ((si->lambdas = realloc(si->lambdas,
			(lam_ind+1)*sizeof(lambda))) == NULL)
	    return 6;		/* realloc fail lam > 0 */
	if ((table_status=realloc(table_status,(lam_ind+1)*sizeof(int)))==NULL)
	    return 6;
    }

    si->lambdas[lam_ind].name = strdup(yytext);
    si->lambdas[lam_ind].corr_table = NULL;
    si->lambdas[lam_ind].corr_kind = None;
    table_status[lam_ind] = 0;		/* don't know yet */

    return 0;
}



/*
** for both setup_we_table and setup_sn_table
** 
** The 'lambdas' section of the file has already told us which of
** the num_lambdas lambdas require a table. At this point, the data
** structure slot for the current (about to read) table is unallocated; 
** first make sure that we expect a table for 'this' slot; then
** allocate the table and it's cosine vectors. Finally, indicate, via
** the table_status vector, that we've found a table we've been looking for.
*/
int setup_we_table(void)
{
    sscanf(yytext, "%*2c%d", &we_ind);

    if (we_ind < 1 || we_ind > si->num_lambdas)
	return 8;	/* we index out of range */
    we_ind--;	/* count from 0... */

    /* are we looking for this table? */
    if (!(table_status[we_ind] & 1))
	return 21;	/* found a table for a lambda that doesn't need 1 */
    if (table_status[we_ind] & 2)
	return 22;	/* too many we tables for this channel */

    if (!(table_status[we_ind] & 6)) {		/* make a new corr_table */
	if ((si->lambdas[we_ind].corr_table=malloc(sizeof(response))) == NULL)
	    return 10;	/* can't alloc corr_table */
	si->lambdas[we_ind].corr_table->sn = NULL;
    }
    if ((si->lambdas[we_ind].corr_table->we =
			malloc(181*sizeof(double))) == NULL)
	return 9;	/* we table alloc fail */

    we_i = 0;
    table_status[we_ind] += 2;		/* cool */

    return 0;
}




int setup_sn_table(void)
{
    sscanf(yytext, "%*2c%d", &sn_ind);

    if (sn_ind < 1 || sn_ind > si->num_lambdas)
	return 11;	/* sn index out of range */
    sn_ind--;

    /* is this table required? */
    if (!(table_status[sn_ind] & 1))
	return 23;	/* sn table for channel not requesting one */
    if (table_status[sn_ind] & 4)
	return 24;	/* too many s-n table for this channel */

    if (!(table_status[sn_ind] & 6)) {		/* make a new corr_table */
	if ((si->lambdas[sn_ind].corr_table = malloc(sizeof(response))) == NULL)
	    return 12;	/* can't alloc corr table */
	si->lambdas[sn_ind].corr_table->we = NULL;
    }
    if ((si->lambdas[sn_ind].corr_table->sn =
			malloc(181*sizeof(double))) == NULL)
	return 13;	/* sn table alloc fail */

    sn_i = 0;
    table_status[sn_ind] += 4;

    return 0;
}




int new_we_number(void)
{
    if (we_i > 180)
	return 16;	/* too many entries in we table */

    si->lambdas[we_ind].corr_table->we[we_i] = my_strtod(yytext, NULL);

    we_i++;

    return 0;
}




int new_sn_number(void)
{
    if (sn_i > 180)
	return 17;	/* too many entries in sn table */

    si->lambdas[sn_ind].corr_table->sn[sn_i] = my_strtod(yytext, NULL);

    sn_i++;

    return 0;
}





int finish_tables(void)
{
    int		i, tables = 0;

    for (i=0; i<si->num_lambdas; i++)
	switch (table_status[i]) {
	case 7:
	    tables++;
	    /* fallthrough */
	case 0:		/* no table requsted */
	    break;
	case 1:
	    return 20;	/* no tables for requested channel */
	case 3:
	    return 14;	/* found we but not sn */
	case 5:
	    return 15;	/* found sn but not we */
	default:	/* "impossible" */
	    return 26;	/* unknown table mismatch */
	}

    if (tables < si->num_filt)
	return 25;	/* fewer tables than filters */

    if (table_status != NULL)
	free(table_status);

    return 0;
}

int check_var(char *str)
{
    char *p, *q;

    p = str;
    q = yytext;

    while(*p) {
	if( ! (*p == *q || (*p-32) == *q || (*q-32) == *p) )
		return 0;
	p++;q++;
    }

    return 1;
}

