#include <stdio.h>

#include "rsrlib.h"

int main(void)
{
    char	buf[512];
    int		len;
    double	d;

    d = 1.1;
    while (d > 0.09) {
	len = fmt4(d, buf);
	printf("%.4f %s = %d\n", d, buf, len);
	d -= 0.0001;
    }

    return 0;
}
