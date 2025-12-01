CC	= gcc
CFLAGS	= -g
#CFLAGS	= -O
#CFLAGS = -O -DSUNOS
OBJS	= rsrlibc.o sstoa.o fmt4.o

librsr.a: $(OBJS)
	ar r librsr.a $(OBJS)

clean:
	/bin/rm -f $(OBJS) librsr.a

#
#.c.o:
#	$(CC) $(CFLAGS) -c $<
#
#
