CC	= cc
CFLAGS = -O -I.. -Wno-error=implicit-function-declaration # -DTEST2

# LEXFLAGS:  -i means case insensitive lexer
#	     -p means report efficiency concerns
#	     -I means, well, see man page
LEX	= flex 
LEXFLAGS = -i

YACC	= bison
YFLAGS	=

OBJS	= cos.o lex.yy.o coscorr.tab.o mystrtod.o
LIBFLAGS = -lm

# coscorr depends on the source and the lexer objects
libsi.a:	$(OBJS)
	ar r libsi.a $(OBJS)
	ranlib libsi.a

clean:
	/bin/rm -f $(OBJS) libsi.a

cos:    $(OBJS)
	$(CC) $(CFLAGS) -DTEST -c cos.c
	$(CC) $(CFLAGS) -o cos $(OBJS) $(LIBFLAGS)

# lexer object depends on the lex output and the yacc .h file
lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

# lex output depends on the lexer
lex.yy.c: coscorr.l
	$(LEX) $(LEXFLAGS) coscorr.l

coscorr.tab.o:	coscorr.tab.c
	$(CC) $(CFLAGS) -c $<
	
coscorr.tab.c:	coscorr.y
	$(YACC) $(YFLAGS) coscorr.y
