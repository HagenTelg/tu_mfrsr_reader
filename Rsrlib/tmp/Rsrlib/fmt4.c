#include <stdio.h>
#include <string.h>
#include <ctype.h>


#define SIG	4


/*
** written by Jim. Indented by Rob.
*/
/*
* all we want is 'SIG' digits of significance.
*/
int fmt4(double x, char *str)
{
    int		len, d, i;
    char	temp[256], *s, *t, seen_dot=0;

    /* if x is zero, make it so */
    if(x == 0) {
	strcpy(str, "0");
	return 1;
    }

    s = str;

    if(x < 0) {
	*s++ = '-';
	x = -x;
    }

    sprintf(temp, "%f", x);  /* guaranteed to be positive */

    if(!isdigit(*temp)) {  /* something - probably a Nan or Inf */
	strcpy(str, temp);
	return strlen(str);
    }

    if(x < 1.0) {
	/* in this case we skip by the "0." and just copy SIG numbers */
	*s++ = '0';
	*s++ = '.';
	t = temp+2;

	/*
	 * this converts 0.00004 to 0. rather than 0
	 * which is, perhaps a good thing
	 */
	for (i=0; i<SIG && t[i] != '\0'; i++) 
	    *s++ = t[i];

	while(*(s-1) == '0')
	    s--;  /* lop off trailing 0's */

	*s = '\0';

	return 1;
    }
	

    /* abs(input) >= 1.0 */
    len = strlen(temp);

    for(d=0,i=0;i<len;i++) {
	if(temp[i] == '.') {
	    if(d > SIG)
		break;
	    seen_dot = 1;
	    *s++ = temp[i];
	} else {
	    if(++d > SIG) {
		if(seen_dot)
		    break;
		else
		    *s++ = '0';
	    } else
		*s++ = temp[i];
	}
    }

  while(*(s-1) == '0')
      s--;  /* lop off trailing 0's */
  if(*(s-1) == '.')
      s--;  /* lop off trailing '.' if necessary */

  *s = '\0';

  return 1;
}







#ifdef TEST


int main(void)
{
    char	buf[512];
    int		len;
    double	d;

    d = 14.0;
    len = fmt4(d, buf);
    printf("%s = %d\n", buf, len);

    d = 0.0;
    len = fmt4(d, buf);
    printf("%s = %d\n", buf, len);

    d = 1.356789;
    len = fmt4(d, buf);
    printf("%s = %d\n", buf, len);

    return 0;
}


#endif
