#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "tu.h"
#include "rsrlib.h"
#include "calib.h"


static int	line_no = 0;
char		ebuf[1024];
void		delete_calib(cal_t *);
int		ig_strcmp(char *, char *),
		split(char *, char **, int, char *);

/*
** read a calibration file and return a pointer to the first element
** in a chronologically sorted list of calibs.
**
** see "calib.txt" for a description of the calibration file.
**
*/
cal_t *read_cal(char *fnm)
{
    FILE	*f;
    void	print_cal_t(cal_t *),
		sort_channels(cal_t *);
    int		calib_check(cal_t *);
    cal_t	*clist, *read_cal_file(FILE *),
		*c;

    if ((f = fopen(fnm, "r")) == NULL)
	return NULL;

    clist = read_cal_file(f);

    fclose(f);

    /*
    ** do some basic consistency checks to make sure the
    ** content of the file is sensible
    */
    if (clist != NULL)
	if (!calib_check(clist)) {
	    delete_calib(clist);
	    clist = NULL;
	}

    /*
    ** sort each cal_el's channels
    */
    if (clist != NULL)
	sort_channels(clist);

#ifdef DEBUG
    for (c=clist; c != NULL; c=c->next)
	print_cal_t(c);
#endif

    return clist;
}





cal_t *read_cal_file(FILE *f)
{
    char	buf[8192], *sp_buf[16];
    int		rv, got,
		next_cal_record(FILE *, char *),
		new_cal_el(cal_t *, char *s[16]);
    cal_t	*cal = NULL, *tc = NULL,
		*new_cal_t(char *s[16], int),
		*insert_cal(cal_t *, cal_t *);
    void	cleanup_string(char *);

    while ((rv = next_cal_record(f, buf)) == 1) {
	if ((got = split(buf, sp_buf, 16, "|")) != 3 && got != 4) {
	    sprintf(buf, "Error: calibration file line %d\n"
		"--> There are %d fields on this line\n", line_no, got);
	    tu_msg(buf);
	    delete_calib(tc);
	    delete_calib(cal);
	    return NULL;
	}

	cleanup_string(sp_buf[0]);
	cleanup_string(sp_buf[1]);
	cleanup_string(sp_buf[2]);
	if (got == 3 && ig_strcmp(sp_buf[0], "calibration") == 0) {
	    cal = insert_cal(cal, tc);
	    if ((tc = new_cal_t(sp_buf, line_no)) == NULL) {
		delete_calib(cal);
		return NULL;
	    }
	} else if (got == 4) {
	    cleanup_string(sp_buf[3]);
	    if (!new_cal_el(tc, sp_buf)) {
		sprintf(ebuf, "Error in calib file line %d\n", line_no);
		tu_msg(ebuf);
		delete_calib(cal);
		return NULL;
	    }
	} else {
	    sprintf(buf, "Error: calibration file line %d\n"
		"--> Uncategorizable line - giving up (%d)\n", line_no, got);
	    tu_msg(buf);
	    delete_calib(tc);
	    delete_calib(cal);
	    return NULL;
	}
    }

    if (rv != 0) {		/* last line was coninuation...error */
	delete_calib(cal);
	return NULL;
    }

    /* now insert the last one in the chain */
    cal = insert_cal(cal, tc);

    return cal;
}




/*
** read the next record from the calibration file; this is normally
** a single line, but we have to support continuations.
** Return
**    1 - got another line
**    0 - no more lines
**   -1 - last line in file had a continuation indicator; error.
*/
int next_cal_record(FILE *f, char *buf)
{
    char	*p, *s,
		*cleanup_line(char *);
    void	cleanup_string(char *);
    int		pending, empty_string(char *);

    p = buf;
    pending = 0;

    do {
	if (fgets(p, 512, f) == NULL) {
	    if (!pending)	/* no pending continuation */
		return 0;
	    else {
		tu_msg("Unfinished continuation in calib file, last line");
		return -1;
	    }
	}

	line_no++;

	/* remove comments and/or newlines */
	cleanup_line(p);
	/* remove leading and trailing whitespace */
	cleanup_string(p);

	if (empty_string(p))
	    if (!pending)
		continue;
	    else {
		/* give a warning */
		sprintf(ebuf, "Warning: Cal file line %d: "
			"unfinished continuation", line_no);
	  	tu_msg(ebuf);
		return 1;
	    }

	/* find last character; if '\' then continuation */
	s = strchr(p, '\0');
	if (*--s == '\\') {
	    pending++;
	    p = s;
	    continue;
	}

	return 1;
    } while (1);
}






/*
** this split-up line should be for a channel; break it up and add it
** to 'ct'; if errors, return 0, otherwise, return 1;
*/
int new_cal_el(cal_t *ct, char *buf[16])
{
    cal_el	*c;
    char	*sp_buf[32];
    int		i;

    ct->cals = realloc(ct->cals, (ct->n_cals + 1)*sizeof(cal_el));
    if (ct->cals == NULL) {
	sprintf(ebuf, "Error: Calibration file: No memory for el %d\n",
			ct->n_cals + 1);
	tu_msg(ebuf);
	return 0;
    }

    c = ct->cals + ct->n_cals;
    c->name = strdup(buf[1]);
    c->units = strdup(buf[2]);
    if (sscanf(buf[0], "%d", &(c->index)) != 1) {
	sprintf(ebuf, "Error: Calibration: channel (%s) should be an int\n",
			buf[0]);
	tu_msg(ebuf);
	return 0;
    }
    if ((c->order = split(buf[3], sp_buf, 32, "")) < 1) {
	sprintf(ebuf, "Error: Calibration: No coefficients\n");
	tu_msg(ebuf);
	return 0;
    }
    if ((c->coefs = malloc(c->order*sizeof(double))) == NULL) {
	sprintf(ebuf, "Error: Calibration: Memory alloc failure\n");
	tu_msg(ebuf);
	return 0;
    }
    for (i=0; i<c->order; i++)
	if (sscanf(sp_buf[i], "%lf", c->coefs + i) != 1) {
	    sprintf(ebuf, "Error: Calibration: can't convert (%s) to"
			" a float\n", sp_buf[i]);
	    tu_msg(ebuf);
	    return 0;
	}

    ct->n_cals++;
    return 1;
}








/*
** verify that this line is a good calibration header, then
** allocate a new cal_t and copy the header stuff to it. return NULL
** if anything is wrong.
*/
cal_t *new_cal_t(char *buf[16], int line_no)
{
    char	ebuf[128];
    cal_t	*ct;
    time_t	tm;
    int		str_to_time(char *, time_t *);

    if (!str_to_time(buf[1], &tm)) {
	sprintf(ebuf, "Error calib file line %d:\n"
		"Date field (%s) undecipherable\n", line_no, buf[1]);
	tu_msg(ebuf);
	return NULL;
    }

    if ((ct = malloc(sizeof(cal_t))) == NULL) {
	sprintf(ebuf, "Error calib file line %d:\n"
		"ran out of memory loading calibrations\n", line_no);
	tu_msg(ebuf);
	return NULL;
    }

    ct->date = tm;
    ct->n_cals = 0;
    ct->id_string = strdup(buf[2]);
    ct->source_line_no = line_no;
    ct->cals = NULL;
    ct->next = NULL;

    return ct;
}






/*
** convert a string with (supposedly) an acceptable date representation
** into a time_t;
**
** s can be of the form
** mon dom yr	 - mon is "jan" "feb", etc; NOT "1", "2", etc.
** dom mon yr
** mm/dd/yy
*/
int str_to_time(char *s, time_t *tt)
{
    char	*sp_buf[3], cp[512];
    int		style, i, mo, dom, yr;
    extern int	errno;
    struct tm	tm;
    time_t	t;
    static const char
		*mons[12] = {"jan", "feb", "mar", "apr", "may", "jun",
				"jul", "aug", "sep", "oct", "nov", "dec"};

    /* these are to shut up warnings about possibly unset variables */
    yr = 0;
    mo = 0;
    dom = 0;

    strcpy(cp, s);

    if (split(cp, sp_buf, 3, "/") == 3) {
	style = 0;	/* ie, mm/dd/yy */
    } else {
	strcpy(cp, s);
	if (split(cp, sp_buf, 3, "") == 3) {
	    style = -1;
	    for (i=0; i<12; i++) {
		if (ig_strcmp(sp_buf[0], (char *) mons[i]) == 0) {
		    mo = i + 1;
		    style = 1;	/* ie, jan 1 95 */
		    break;
		}
		if (ig_strcmp(sp_buf[1], (char *) mons[i]) == 0) {
		    mo = i + 1;
		    style = 2;	/* ie, 1 jan 95 */
		    break;
		}
	    }
	    if (style == -1)
		return 0;
	} else {
	    return 0;
	}
    }

    errno = 0;
    yr = atoi(sp_buf[2]);
    switch (style) {
    case 0:
	mo = atoi(sp_buf[0]);
	dom = atoi(sp_buf[1]);
	break;
    case 1:
	dom = atoi(sp_buf[1]);
	break;
    case 2:
	dom = atoi(sp_buf[0]);
	break;
    }
    if (errno != 0)
	return 0;

    /* check if mo, dom, and yr are reasonable */
    if (mo < 1 || mo > 12)
	return 0;
    if (dom < 1 || dom > 31)
	return 0;
    if (yr > 2000 || (yr > 99 && yr < 1980) || yr < 80)
	return 0;

    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;
    tm.tm_mday = dom;
    tm.tm_mon = mo - 1;
    if ((tm.tm_year = yr % 100) < 20)
	tm.tm_year += 100;
    tm.tm_isdst = 0;
    t= mktime(&tm);
    *tt = t;

    return 1;
}








/*
** insert the "new_el" into the list with "head" at its head
*/
cal_t *insert_cal(cal_t *head, cal_t *new_el)
{
    cal_t	*prev = NULL, *curr,
		*append(cal_t *, cal_t *);

    if (head == NULL)
	return new_el;

    if (new_el->date < head->date) {
	new_el->next = head;
	return new_el;
    }

    for (curr=head; curr != NULL; prev=curr, curr=curr->next)
	if (new_el->date < curr->date)
	    break;

    (void) append(prev, new_el);

    return head;
}







/*
** insert p2 in the list after p1; return
*/
cal_t *append(cal_t *p1, cal_t *p2)
{
    if (p2 == NULL)
	return p1;
    if (p1 == NULL)
	return p2;

    p2->next = p1->next;
    p1->next = p2;

    return p1;
}






/*
** check each member of the list of cal_t-s to make sure they're
** ok; if any are bad, return 0, else return 1;
*/
int calib_check(cal_t *c)
{
    cal_t	*tc;
    int		check_1_cal(cal_t *);

    for (tc=c; tc != NULL; tc=tc->next)
	if (!check_1_cal(tc))
	    return 0;

    return 1;
}




void cal_err(cal_t *c, char *s)
{
    sprintf(ebuf, "Error: calibration file entry starting on line %d",
			    c->source_line_no);
    tu_msg(ebuf);
    sprintf(ebuf, " --> %s", s);
    tu_msg(ebuf);
}



static const char	*plur[4] = {"st", "nd", "rd", "th"};

/*
** check this single cal_t return 1 if it's OK, 0 otherwise
*/
int check_1_cal(cal_t *c)
{
    char	*list, s[128];
    int		i, idx;

    if (c->n_cals < 1) {
	cal_err(c, "Calibration must have at least 1 channel");
	return 0;
    }
    if (c->n_cals > 64) {
	cal_err(c, "Calibration has too many channels...");
	return 0;
    }

    if ((list = malloc(c->n_cals)) == NULL) {
	cal_err(c, "Memory allocation failure checking calibrations");
	return 0;
    }

    for (i=0; i<c->n_cals; i++)
	list[i] = 0;

    for (i=0; i<c->n_cals; i++) {
	idx = c->cals[i].index - 1;
	if (idx < 0 || idx >= c->n_cals) {
	    sprintf(s, "Cal %d%s channel index is bad (%d not in [1..%d])", 
		    i+1, plur[i>3?3:i], c->cals[i].index, c->n_cals);
	    cal_err(c, s);
	    return 0;
	}
	list[idx]++;
    }

    for (i=0; i<c->n_cals; i++)
	if (list[i] != 1) {
	    sprintf(s, "Cal channel %d has %d referents", i+1, list[i]);
	    cal_err(c, s);
	    return 0;
	}

    free(list);

    for (i=0; i<c->n_cals; i++)
	if (c->cals[i].order < 1 || c->cals[i].order > 16) {
	    sprintf(s, "Cal channel %d function has order %d!", i+1,
			c->cals[i].order);
	    cal_err(c, s);
	    return 0;
	}

    return 1;
}







void sort_channels(cal_t *c)
{
    int		cal_el_cmp(const void *, const void *);

    while (c != NULL) {
	qsort(c->cals, c->n_cals, sizeof(cal_el), cal_el_cmp);
	c = c->next;
    }
}



int cal_el_cmp(const void *v1, const void *v2)
{
    const cal_el	*c1 = (const cal_el *) v1,
			*c2 = (const cal_el *) v2;

    return c1->index - c2->index;
}










void print_cal_t(cal_t *c)
{
    int		i, j;
    char	buf[512];
    struct tm	*t;

    t = localtime(&(c->date));
    strftime(buf, 512, "%b %e %Y %T", t);
    printf("Date: %s ID: %s Channels: %d LIne: %d\n", buf, c->id_string,
		c->n_cals, c->source_line_no);
    for (i=0; i<c->n_cals; i++) {
	printf("%d: Ch %d: '%s'  Units: '%s'\n    %d coeffs:",
			i, c->cals[i].index,
			c->cals[i].name, c->cals[i].units, c->cals[i].order);
	for (j=0; j<c->cals[i].order; j++)
	    printf(" %.4f", c->cals[i].coefs[j]);
	puts("");
    }
}

















/*
** End of calibration file input and manipulation functions.
**
**88*****888***********888888*888888**********88888888************88888*/





/*
** delete a cal_t
*/
void delete_calib(cal_t *c)
{
    int		i;

    if (c == NULL)
	return;

    delete_calib(c->next);

    for (i=0; i<c->n_cals; i++) {
	free(c->cals[i].name);
	free(c->cals[i].units);
	free(c->cals[i].coefs);
    }

    free(c->id_string);
    free(c->cals);

    free(c);
}






typedef struct {
    cal_el	*c1, *c2;
    double	wt1, wt2;
} interp_t;





/*
** Actually perform a calibration on an input record
*/
void calibrate(RSR_file *rf, cal_t *cal, double *data)
{
    int		i, n,
		gen_cals(RSR_file *, cal_t *, interp_t *);
    double	t1, t2,
		weight(interp_t *, double, double),
		compute_poly(cal_el *, signed short);
    interp_t	cals[64], *it;
    static int	night_checked = 0, day_checked = 0;

    if (cal == NULL || data == NULL)
	return;
	
    if ((n = gen_cals(rf, cal, cals)) == 0) {
	tu_msg("Calibration error: input record preceeds all calibrations");
	return;
    }

    if (n < rf->record->n_data) {
	tu_msg("Cal error: too few cal channels for input record");
	return;
    } else if (n > rf->record->n_data) {
	/* if it's daytime, supposed to put a warning here */
    }

    if (rf->record->n_data == n)	/* daytime */
	it = cals;
    else if (rf->inst_type == Type2)	/* nighttime Type2 */
	it = cals;
    else				/* night time pre-Type2 */
	it = cals + (rf->rec_n_day - rf->rec_n_nite);

    for (i=0; i<rf->record->n_data; i++) {
	if (it[i].c2 == NULL) {
	    data[i] = compute_poly(it[i].c1, data[i]);
	} else {
	    t1 = compute_poly(it[i].c1, data[i]);
	    t2 = compute_poly(it[i].c2, data[i]);
	    data[i] = weight(it + i, t1, t2);
	}
    }
}







/*
** figure out what the actual calibrations used for the current date
** should be. return 0 if the date is before any in the cal list,
** otherwise return 1. set the elements in 'cals' to be pointers
** to the previous and next calibrations for each channel.
** if the current date is after the last cal in the file, then
** the 'wt2' element of each of 'cals' will be 0.0.
*/
int gen_cals(RSR_file *rf, cal_t *clist, interp_t *cals)
{
    extern struct tm	*record_tm;
    time_t		record_t;
    cal_el		*c1, *c2;
    cal_t		*c;
    int			i;

    record_t = mktime(record_tm);

    if (record_t < clist->date)
	return 0;

    for (c=clist; c->next != NULL; c=c->next)
	if (record_t < c->next->date)
	    break;

    for (i=0; i<c->n_cals; i++) {
	if (c->next == NULL) {
	    cals[i].wt1 = 1.0;
	    cals[i].wt2 = 0.0;
	    cals[i].c1 = c->cals + i;
	    cals[i].c2 = NULL;
	} else {
	    cals[i].wt1 = 1.0 -
			(1.0*record_t - c->date)/(c->next->date - c->date);
	    cals[i].wt2 = 1.0 - cals[i].wt1;
	    cals[i].c1 = c->cals + i;
	    cals[i].c2 = c->next->cals + i;
	}
    }

    return c->n_cals;
}






double compute_poly(cal_el *c, signed short s)
{
    int		i, n;
    double	t, ex;

    if (c == NULL)
	return 0.0;

    /* if only 1 coeff, it's a bias, not a constant */
    if ((n = c->order) == 1)
	return s + c->coefs[0];

    t = c->coefs[n-1];
    ex = s;
    for (i=n-2; i>=0; i--) {
	t += ex * c->coefs[i];
	ex *= s;
    }

    return t;
}





double weight(interp_t *it, double t1, double t2)
{
    return t1*it->wt1 + t2*it->wt2;
}







/*
typedef struct {
    char	*name, *units;
    int		index, order;
    double	*coefs;
} cal_el;

typedef struct c_t {
    time_t	date;
    int		n_cals;
    char	*id_string;
    cal_el	*cals;
    struct c_t	*next;
} cal_t;
*/
