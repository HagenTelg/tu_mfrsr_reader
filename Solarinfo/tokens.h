#ifndef	TOKENS_H
#define	TOKENS_H

#ifndef YYSTYPE
#define YYSTYPE int
#endif
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
#define	TOK_HEX		284


extern YYSTYPE yylval;

#endif	/* TOKENS_H */
