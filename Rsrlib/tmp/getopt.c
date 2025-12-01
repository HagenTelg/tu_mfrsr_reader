#ifdef __MSDOS__

/*
**      Copyright (c) 1986,87,88 by Borland International Inc.
**      All Rights Reserved.
**
**	Parse the command line options, System V style. From Borland.
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dos.h>

int     optind  = 1;    /* index of which argument is next      */
char   *optarg;         /* pointer to argument of current option */
int     opterr  = 1;    /* allow error message  */

static	char   *letP = NULL;	/* remember next option char's location */
static	char	SW = 0;		/* DOS switch character, either '-' or '/' */


int getopt(int argc, char *argv[], char *optionS)
{
	unsigned char ch;
	char *optP;

	/*
	** the '/' switch character is for dolts and sissies
	*/
	SW = '-';
	if (argc > optind) {
		if (letP == NULL) {
			if ((letP = argv[optind]) == NULL ||
				*(letP++) != SW)  goto gopEOF;
			if (*letP == SW) {
				optind++;  goto gopEOF;
			}
		}
		if (0 == (ch = *(letP++))) {
			optind++;  goto gopEOF;
		}
		if (':' == ch  ||  (optP = strchr(optionS, ch)) == NULL)  
			goto gopError;
		if (':' == *(++optP)) {
			optind++;
			if (0 == *letP) {
				if (argc <= optind)  goto  gopError;
				letP = argv[optind++];
			}
			optarg = letP;
			letP = NULL;
		} else {
			if (0 == *letP) {
				optind++;
				letP = NULL;
			}
			optarg = NULL;
		}
		return ch;
	}
gopEOF:
	optarg = letP = NULL;  
	return EOF;
 
gopError:
	optarg = NULL;
	errno  = EINVAL;
	/*
	if (opterr)
		perror ("get command line option");
	*/
	return ('?');
}

#endif
