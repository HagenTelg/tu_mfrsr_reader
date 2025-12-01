#include <string.h>

int sstoa(signed short s, char *buf)
{
    static const char	*st1[100] = {
	    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
	    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
	    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
	    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
	    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
	    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
	    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99"};
    static const char	*st2[100] = {
	    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
	    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
	    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
	    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
	    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
	    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
	    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
	    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99"};
    int			dv, mo, cc = 0;

    if (s >= 0 && s < 100) {
	strcpy(buf, st1[s]);
	return 1;
    }

    if (s < -9999 || s > 9999) {
	buf[0] = 'N';
	buf[1] = 'A';
	buf[2] = '\0';
	return 2;
    }

    if (s < 0) {
	buf[cc++] = '-';
	s = ~s + 1;
    }

    dv = s/100;
    if (dv > 0) {
	if (dv < 10) {
	    buf[cc++] = st1[dv][0];
	} else {
	    buf[cc++] = st1[dv][0];
	    buf[cc++] = st1[dv][1];
	}
    }

    mo = s % 100;
    if (dv) {
	buf[cc++] = st2[mo][0];
	buf[cc++] = st2[mo][1];
    } else {
	if (mo < 10) {
	    buf[cc++] = st1[mo][0];
	} else {
	    buf[cc++] = st1[mo][0];
	    buf[cc++] = st1[mo][1];
	}
    }
    buf[cc] = '\0';

    return cc;
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    signed short        ss;
    int                 sstoa(signed short, char *);
    long		i;
    char                buf[64];

    for (i=0; i<1000000; i++) {
	ss = rand() & 0xffff;
	if (ss < -9999 || ss > 9999)
	    continue;

	(void) sstoa(ss, buf);
    }
} 

#ifdef PRINTF
int sstoa(signed short s, char *buf)
{
    return sprintf(buf, "%hd", s);
}
#elif defined(DIV1)
#elif defined(SUBS)
int sstoa(signed short s, char *buf)
{
    static const int	subs[4] = {10000, 1000, 100, 10};
    int			cnt, log, cc = 0, started = 0;

    if (s == 0) {
	buf[0] = '0';
	buf[1] = '\0';
	return 1;
    }

    if (s < 0) {
	buf[cc++] = '-';
	s = ~s + 1;
    }

    for (log=0; log<4; log++) {
	for (cnt=0; s >= subs[log]; cnt++)
	    s -= subs[log];

	if (started)
	    buf[cc++] = cnt + '0';
	else if (cnt > 0) {
	    started++;
	    buf[cc++] = cnt + '0';
	}
    }
    buf[cc++] = s + '0';
    buf[cc] = '\0';

    return cc;
}
#else
int sstoa(signed short s, char *buf)
{
    int		is_neg = 0, cc = 5, bytes;
    char	cbuf[16];

    if (s == 0) {
	buf[0] = '0';
	buf[1] = '\0';
	return 1;
    }

    /* memset(cbuf, ' ', 6); */
    cbuf[6] = '\0';

    if (s < 0) {
	is_neg++;
	s = ~s + 1;
    }

    do {
	cbuf[cc--] = (s % 10) + '0';
	s /= 10;
    } while (s);

    if (is_neg)
	cbuf[cc--] = '-';
    bytes = 7 - cc - 1;
    memcpy(buf, cbuf+cc+1, bytes);

    return bytes;
}
#endif

#endif
