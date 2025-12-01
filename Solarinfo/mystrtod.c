#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/*
** 4/5/96 rs
** This has got to be able to do hex constants of the form "$hh"
**
** 6/24/96 rs
** Jim found and fixed this once, but it apparently never get propagated:
** sometimes global and direct turn into -1 for no apparent reason,
** and it got traced to the test (*p == '$') below, which used to be '='.
**
*/

double my_strtod(char *s, char **endp)
{
    char	*dotp, *p;
    int		ipart, fpart, flen;
    double	pow_ten(int),
		hextod(char *);

    /*
    ** check if this is a hex thingy - leading '$' followed by hex
    */
    for (p=s; *p != '\0' && (*p == ' ' || *p == '\t'); p++) {
    }
    if (*p == '\0')
	return 0.0;
    else if (*p == '$')
	return hextod(p+1);

#ifndef sparc
    return strtod(s, endp);
#endif

#ifdef DEBUG
fprintf(stderr, "my_strtod: `%s'", s);
#endif

    if (endp != NULL) {
#ifdef DEBUG
fputs("return system strtod\n", stderr);
#endif
	return strtod(s, endp);
    }

    if ((dotp = strchr(s, '.')) == NULL) {
#ifdef DEBUG
fputs("return system atoi\n", stderr);
#endif
	return (double) atoi(s);
    }

    if (dotp != s)
	ipart = atoi(s);
    else
	ipart = 0;
    if ((flen = strlen(dotp + 1)) < 1) {
#ifdef DEBUG
fputs("ends with dot, returning system atoi\n", stderr);
#endif
	return (double) ipart;
    } else {
	fpart = atoi(dotp + 1);
#ifdef DEBUG
fprintf(stderr, "%d + %d\n", ipart, fpart);
#endif
	return ipart + fpart/pow_ten(flen);
    }
}




double pow_ten(int p)
{
    static const double	powtab[10] = {1.0,
					10.0,
					100.0,
					1000.0,
					10000.0,
					100000.0,
					1000000.0,
					10000000.0,
					100000000.0,
					1000000000.0};

    if (p < 0 || p > 9)
	return pow(10.0, (double) p);
    else
	return powtab[p];
}




/*
** s should point to some hex characters
*/
double hextod(char *s)
{
    int		h;

    if (sscanf(s, "%x", &h) != 1)
	return 0.0;

    return (double) h;
}





#ifdef TEST
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    char        buf[512];
    int         i;
    double      d;
    time_t      t1, t2;

    sprintf(buf, "15.412365");

    t1 = time(NULL);
    for (i=0; i<100000; i++)
        d = my_strtod(buf, NULL);
    t2 = time(NULL);
    printf("%d %f\n", t2 - t1, d);


    t1 = time(NULL);
    for (i=0; i<100000; i++)
        d = strtod(buf, NULL);
    t2 = time(NULL);
    printf("%d %f\n", t2 - t1, d);

    return 0;
}

#endif








