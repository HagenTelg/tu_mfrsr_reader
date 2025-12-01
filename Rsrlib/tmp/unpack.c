#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tu.h"
#include "rsrlib.h"
#include "sunae.h"
#include "cosdefs.h"
#include "calib.h"

static char	buf[1024];
static ae_pack	aep;
static double	tst;
static int	sunae_called;
static void	call_sunae(RSR_file *);

/*
** do all the work. return 0 for most circumstances, indicating
** that it did what it was supposed to. if there are fatal errors,
** return the (non-zero) code indicating which.
**
** Code	Meaning
**   1	can't open output file.
**   2  cosine correction file doesn't exist or is errroneous
**   3	cosine correction file doesn't match an input file
**   4  calibration file doesn't exist or is malformed
**   5  other calibration failure
**   6  memory allocation failure
**
**
** 3/19/96 rs
** Since cosine correcting and calibrating can change the dynamic range
** of the data, we have to carry them around as floating-point quantities.
** However, the inherent accuracy stays the same! Create a new vector
** of doubles to hold the data after getting a new record, and apply
** all changes to this vector; print it as well. Jim is writing a new
** function to print a double with 4 digits of accuracy in all cases.
**
** 4/29/96 rs
** Using "date=rsr" produced incorrect output. This was caused by printf
** rounding the 'rsr-style' date. Fixed.
**
** 9/23/96 rs
** Confusing and incorrect usage of floating-point and fixed output
** vectors; changed so that a floating point vector is always used
** except when it's a gap record - then it just copies the stuff verbatim.
**
** 10/24/96 rob
** Benoit points out that calibration should logically come before
** cos correction. Switched the 2.
**
** 10/24/96 rob
** Benoit points out that when calibration and cos correction are both
** selected, only calibration is performed. Try to determine the cause
** of this bug. Found: it didn't happen after I switched the order of
** calib/coscorr; the problem was in "calibrate", it used the uncorrected
** data right from the input record.
**
*/
int do_unpack(proc_params *pp)
{
    solarinfo	*si, *read_si(char *, double);
    cal_t	*cal, *read_cal(char *);
    int		compare_si(RSR_file *, solarinfo *);
    void	cosine_correct(RSR_file *, solarinfo *, double *);
    double	*data_vec, *copy_data(RSR_file *);
    RSR_file	*inp;
    RSR_rec	*r;
    FILE	*outf;
    int		i, opened = 0;
    void	emit_header(FILE *, RSR_file *, proc_params *),
		emit_date_time(FILE *, RSR_file *, proc_params *),
		correct_direct(RSR_file *, double *),
		emit_data(FILE *, RSR_file *, proc_params *, double *),
		emit_extra(FILE *, RSR_file *, proc_params *);

    if (pp->out_fnm.u_val.sval == NULL)
	outf = stdout;
    else if ((outf = fopen(pp->out_fnm.u_val.sval, "w")) == NULL) {
	sprintf(buf, "Error: can't open output file '%s'",
				pp->out_fnm.u_val.sval);
	tu_msg(buf);
	return 1;
    }

    for (i=0; i<pp->n_files_to_unpack; i++) {
	if ((inp = rsr_open_file(pp->fnms[i])) == NULL) {
	    sprintf(buf, "Warning: Couldn't open file '%s':", pp->fnms[i]);
	    tu_msg(buf);
	    tu_msg(rsr_last_message());
	    continue;
	}

	opened++;

	/* first pass through - check for cos corr/calib file */
	if (opened == 1) {
	    if (pp->calibration.u_val.sval != NULL) {
		if ((cal = read_cal(pp->calibration.u_val.sval)) == NULL) {
		    sprintf(buf, "Error: Calibration file '%s'",
					    pp->calibration.u_val.sval);
		    tu_msg(buf);
		    return 4;
		}
	    } else {
		cal = NULL;
	    }

	    if (pp->coscor_fnm.u_val.sval != NULL) {
		if ((si = read_si(pp->coscor_fnm.u_val.sval,
					    inp->head->latitude)) == NULL) {
		    sprintf(buf, "Error: cosine (SolarInfo) file '%s'",
					    pp->coscor_fnm.u_val.sval);
		    tu_msg(buf);
		    return 2;
		}
	    } else
		si = NULL;
	}

	/* if there's a coscor file, make sure it matches this rsr file */
	if (si != NULL) {
	    if (!compare_si(inp, si)) {
		sprintf(buf, "Error: SolarInfo file (%s) doesn't correspond"
			    " with RSR file (%s)", pp->coscor_fnm.u_val.sval,
			    pp->fnms[i]);
		tu_msg(buf);
		rsr_file_delete(inp);
		return 3;
	    }
	}

	tu_advise(pp->fnms[i]);
	rsr_print_header(inp, buf, 1024);
	tu_advise(buf);

	emit_header(outf, inp, pp);

	do {
	    if ((r = rsr_next_record(inp)) == NULL) {
		tu_msg(rsr_last_message());
		break;
	    }

	    sunae_called = 0;
	    if (r->n_data >= 0) {
		emit_date_time(outf, inp, pp);
		if (!r->is_gap) {
		    if ((data_vec = copy_data(inp)) == NULL)
			return 6;
		    correct_direct(inp, data_vec);
		    calibrate(inp, cal, data_vec);
		    cosine_correct(inp, si, data_vec);
		    emit_data(outf, inp, pp, data_vec);
		} else
		    emit_data(outf, inp, pp, NULL);

		emit_extra(outf, inp, pp);

		fputc('\n', outf);
	    }
	} while (r->n_data >= 0);

	if(strcmp(rsr_last_message() , "No errors") != 0) 
		tu_msg(rsr_last_message());

	rsr_file_delete(inp);
    }

    fclose(outf);

    sprintf(buf, "%d RSR files opened", opened);
    tu_msg(buf);

    return 0;
}






/*
** this is not static here because i want to look at it from the
** calibration code...
*/
struct tm	*record_tm;



void emit_date_time(FILE *f, RSR_file *inp, proc_params *pp)
{
    static char	*dfmt = NULL, *tfmt;
    void	setup_t_d_fmts(proc_params *, char **, char **);
    time_t	t;

    /*
    ** the first time this function is called, figure out the
    ** printf formats to use for the date and time; we don't want
    ** to have to redo it every pass through.
    */
    if (dfmt == NULL)
	setup_t_d_fmts(pp, &dfmt, &tfmt);

    /* the time kept with each record is a time_t for the end of sample */
    t = inp->record->obs_time;

    /* correct for the user's preference for reporting time */
    switch (pp->report_time.u_val.rep_val) {
    case Start:
	t -= inp->head->avg_period;
	break;
    case Middle:
	t -= inp->head->avg_period/2;
	break;
    case End:
	break;
    }

    /* correct for the current time zone */
    t -= (int) (3600*pp->timezone.u_val.dval + 0.5);
    record_tm = gmtime(&t);


    switch (pp->date_fmt.u_val.date_val) {
    case Doy0:
	fprintf(f, dfmt, record_tm->tm_year, record_tm->tm_yday);
	break;
    case Doy1:
	fprintf(f, dfmt, record_tm->tm_year, record_tm->tm_yday + 1);
	break;
    case Mdy:
	fprintf(f, dfmt, record_tm->tm_mon+1, record_tm->tm_mday, record_tm->tm_year);
	break;
    case Dmy:
	fprintf(f, dfmt, record_tm->tm_mday, record_tm->tm_mon+1, record_tm->tm_year);
	break;
    case Unix:
	fprintf(f, dfmt, t);
	break;
    case Rsr:
	fprintf(f, dfmt, (int) rsr_unix2j(t));	/* lop off the fraction */
	break;
    case Joe:
	fprintf(f, dfmt, rsr_unix2j(t));
	break;
    case Ymd:
	fprintf(f, dfmt, record_tm->tm_year, record_tm->tm_mon+1, record_tm->tm_mday);
	break;
    }

    /* if user asked for unix or joe time, then we're done */
    if (pp->date_fmt.u_val.date_val == Unix ||
			pp->date_fmt.u_val.date_val == Joe)
	return;

    switch (pp->time_fmt.u_val.time_val) {
    case Hms:
	fprintf(f, tfmt, record_tm->tm_hour, record_tm->tm_min, record_tm->tm_sec);
	break;
    case Hours:
	fprintf(f, tfmt, record_tm->tm_hour + record_tm->tm_min/60.0 + record_tm->tm_sec/3600.0);
	break;
    case Daysec:
	fprintf(f, tfmt, 3600L*record_tm->tm_hour + 60*record_tm->tm_min + record_tm->tm_sec);
	break;
    }
}





void setup_t_d_fmts(proc_params *pp, char **dfmt, char **tfmt)
{
    char	*sep, tbuf[32];
    int		fmted;

    /* is the output supposed to be formatted (tabular)? */
    fmted = pp->separator.u_val.sval == NULL;
    if (!fmted)
	sep = pp->separator.u_val.sval;

    switch (pp->date_fmt.u_val.date_val) {
    case Doy0:
    case Doy1:
	if (fmted)
	    strcpy(tbuf, "%2d %3d ");
	else
	    sprintf(tbuf, "%%d%s%%d%s", sep, sep);
	break;
    case Mdy:
    case Dmy:
	if (fmted)
	    strcpy(tbuf, "%2d %2d %2d ");
	else
	    sprintf(tbuf, "%%d%s%%d%s%%d%s", sep, sep, sep);
	break;
    case Ymd:
	if (fmted)
	    strcpy(tbuf, "%02d%02d%02d ");
	else
	    sprintf(tbuf, "%%02d%%02d%%02d%s", sep);
	break;
    case Unix:
	if (fmted)
	    strcpy(tbuf, "%9ld");
	else
	    strcpy(tbuf, "%ld");
	break;
    case Rsr:
	if (fmted)
	    strcpy(tbuf, "%6d ");
	else
	    sprintf(tbuf, "%%d%s", sep);
	break;
    case Joe:
	if (fmted)
	    strcpy(tbuf, "%12.6f");
	else
	    strcpy(tbuf, "%.5f");
	break;
    }
    if ((*dfmt = strdup(tbuf)) == NULL) {
	tu_msg("fatal: date format alloc fail in emit_date_time");
	exit(1);
    }


    switch (pp->time_fmt.u_val.time_val) {
    case Hms:
	if (fmted)
	    strcpy(tbuf, "%2d %2d %2d");
	else
	    sprintf(tbuf, "%%d%s%%d%s%%d", sep, sep);
	break;
    case Hours:
	if (fmted)
	    strcpy(tbuf, "%7.4f");
	else
	    strcpy(tbuf, "%.4f");
	break;
    case Daysec:
	if (fmted)
	    strcpy(tbuf, "%5ld");
	else
	    strcpy(tbuf, "%ld");
	break;
    }
    if ((*tfmt = strdup(tbuf)) == NULL) {
	tu_msg("fatal: time format alloc fail in emit_date_time");
	exit(1);
    }
}






/*
** For type2 files, the RSR channels appear after the others,
** but in pre-type2 files, the RSR channels appear BEFORE the others.
**
** Now that we have cal/coscor data, we have to be able to print
** with more variety.
*/
void emit_data(FILE *f, RSR_file *r, proc_params *pp, double *vec)
{
    int			i, fmted = 0, len;
    char		*sep, cvted[256];
    static const char	*blnk[6] = {"     ", "    ", "   ", "  ", " ", " "};

    if ((sep = pp->separator.u_val.sval) == NULL)
	fmted = 1;

    /* check for a gap record value */
    if (r->record->n_data > 0 && r->record->is_gap) {
	strcpy(cvted, pp->gap.u_val.sval);
	len = strlen(cvted);
    }

    /* if this is not a type2, and they want placeholders, insert now */
    if (r->inst_type != Type2) {
	if (pp->nighttime.u_val.sval != NULL) {
	    int		j;

	    len = strlen(pp->nighttime.u_val.sval);
	    j = r->rec_n_day - r->record->n_data;
	    for (i=0; i<j; i++) {
		if (fmted)
		    fputs(blnk[len-1], f);
		else
		    fputs(sep, f);
		fputs(pp->nighttime.u_val.sval, f);
	    }
	}
    }

    for (i=0; i<r->record->n_data; i++) {
	if (!r->record->is_gap)
	    if (vec != NULL)
		len = fmt4(vec[i], cvted);
	    else
		len = sstoa(r->record->data[i], cvted);

	if (fmted)
	    if (len < 7)
		fputs(blnk[len-1], f);
	    else
		putc(' ', f);
	else
	    fputs(sep, f);
	fputs(cvted, f);
    }

    /* if this is pre-type2, we are done; if type2, see if night */
    if (r->inst_type == Type2) {
	/* if omitting night values, return now */
	if (pp->nighttime.u_val.sval == NULL)
	    return;

	len = strlen(pp->nighttime.u_val.sval);
	for ( ; i<r->rec_n_day; i++) {
	    if (fmted)
		fputs(blnk[len-1], f);
	    else
		fputs(sep, f);
	    fputs(pp->nighttime.u_val.sval, f);
	}
    }
}






void emit_extra(FILE *f, RSR_file *r, proc_params *pp)
{
    Extra	*ex;
    char	*sep;
    int		fmted = 0;
    time_t	t;
    void	output_extra(double, int, char *, Extra, FILE *);

    if ((ex = pp->extra.u_val.extra_val) == NULL)
	return;

    call_sunae(r);

    if ((sep = pp->separator.u_val.sval) == NULL)
	fmted = 1;

    for ( ; *ex != Done; ex++) {
	switch (*ex) {
	case Tst: output_extra(tst, fmted, sep, *ex, f); break;
	case AzD: output_extra(M_RTOD*aep.az, fmted, sep, *ex, f); break;
	case AzR: output_extra(aep.az, fmted, sep, *ex, f); break;
	case ElD: output_extra(M_RTOD*aep.el, fmted, sep, *ex, f); break;
	case ElR: output_extra(aep.el, fmted, sep, *ex, f); break;
	case ZenD: output_extra(90.0 - M_RTOD*aep.el, fmted, sep, *ex, f); break;
	case ZenR: output_extra(M_PI_2 - aep.el, fmted, sep, *ex, f); break;
	case HaD: output_extra(M_RTOD*aep.ha, fmted, sep, *ex, f); break;
	case HaR: output_extra(aep.ha, fmted, sep, *ex, f); break;
	case DecD: output_extra(M_RTOD*aep.dec, fmted, sep, *ex, f); break;
	case DecR: output_extra(aep.dec, fmted, sep, *ex, f); break;
	case Soldst: output_extra(aep.soldst, fmted, sep, *ex, f); break;
	case Am: output_extra(aep.am, fmted, sep, *ex, f); break;
	}
    }
}



void output_extra(double val, int fmted, char *sep, Extra ex, FILE *f)
{
    switch (ex) {
    case AzD:
    case ElD:
    case ZenD:
    case HaD:
    case DecD:
	if (fmted)
	    fprintf(f, " %7.2f", val);
	else
	    fprintf(f, "%s%.2f", sep, val);
	break;
    case AzR:
    case HaR:
    case ElR:
    case ZenR:
    case DecR:
    case Tst:
	if (fmted)
	    fprintf(f, " %6.3f", val);
	else
	    fprintf(f, "%s%.3f", sep, val);
	break;
    case Soldst:
	if (fmted)
	    fprintf(f, " %6.4f", val);
	else
	    fprintf(f, "%s%.4f", sep, val);
	break;
    case Am:
	if (fmted)
	    fprintf(f, " %7.4f", val);
	else
	    fprintf(f, "%s%.4f", sep, val);
	break;
    }
}





/*
** when sample interval == average interval, new RSRs (post 1-channel)
** do not calculate direct correctly. we must calculate zenith angle
** and divide the reported direct by cos(zen).
*/
void correct_direct(RSR_file *r, double *data)
{
    double	cz;
    int		i, start;

    if (r->head->sample_rate != r->head->avg_period ||
		r->rec_n_nite == r->record->n_data ||
		r->inst_type == Single_Channel ||
		!r->head->band_on ||
		r->head->diodes == 0) {
	return;
    }

    call_sunae(r);
    cz = cos(M_PI_2 - aep.el);

    /*
    ** find all the directs and fix em. single-channel has b...d...g...
    ** starting at the beginning of the data array; all other types
    ** are ordered g...d...b...; type 2 files have rsr data AFTER any other
    ** data, pre-type2 have rsr data before the extra stuff.
    */
    if (r->inst_type == Type2)
	start = r->record->n_data - r->head->diodes;
    else if (r->inst_type == Single_Channel)
	start = 0;
    else
	start = 2*r->head->diodes;

    for (i=start; i<start+r->head->diodes; i++)
	data[i] /= cz;
}



/*
** aep, sunae_called, and tst are file global (static).
*/
void call_sunae(RSR_file *r)
{
    struct tm	*tm;
    time_t	midpt;

    if (sunae_called)
	return;

    midpt = r->record->obs_time + 5;	/* account for arm motion */
    if (r->head->sample_rate != r->head->avg_period) {
	midpt -= r->head->avg_period/2;
	midpt += r->head->sample_rate/2;
    }

    tm = gmtime(&midpt);

    aep.year = tm->tm_year;
    aep.doy = tm->tm_yday + 1;
    aep.hour = tm->tm_hour + tm->tm_min/60.0 + tm->tm_sec/3600.0;
    aep.lat = r->head->latitude;
    aep.lon = -1.0*r->head->longitude;

    sunae_called = 1;

    tst = sunae(&aep);
}





void emit_header(FILE *f, RSR_file *r, proc_params *pp)
{
    static int		done = 0;
    RSR_header		*h;

    if (done != 0)
	return;

    if (pp->header.u_val.ival == 0)
	return;

    done++;

    h = r->head;

    if (r->inst_type == Type2) {
	fprintf(f, "%d %u %u %f %f %d %u %u %lu %lu %d %lu %lu %lu %lu %d\n",
	    h->soft_rev, h->unit_id, h->head_id,
	    h->latitude, h->longitude,
	    h->flags, h->sample_rate, h->avg_period,
	    h->days_1900, h->secs_today, h->diodes, h->raw_daytime,
	    h->raw_all_the_time, h->raw_counter, r->data_length, h->err);
    } else if (r->inst_type == Single_Channel) {
	fprintf(f, "%d %u %f %f %d %u %u %u %u %lu %lu %u %lu\n",
	    h->soft_rev, h->unit_id,
	    h->latitude, h->longitude,
	    h->flags,
	    h->raw_extra, h->raw_bipolar, h->sample_rate, h->avg_period,
	    h->secs_today, h->days_1900, h->rsr_offset,
	    r->data_length);
    } else {	/* 16 or 32 multifilter */
	fprintf(f, "%d %u %f %f %d %u %u %u %u %lu %lu %d %lu\n",
	    h->soft_rev, h->unit_id,
	    h->latitude, h->longitude,
	    h->flags,
	    h->raw_extra, h->raw_bipolar, h->sample_rate, h->avg_period,
	    h->secs_today, h->days_1900, h->diodes,
	    r->data_length);
    }
}






#define	MAX_LAMBDAS	64	/* maximum number of correction tables */


/*
** compare_si
**
** see if it makes sense that 'si' represents 'r'. if ok, return 1, else 0.
*/
int compare_si(RSR_file *r, solarinfo *si)
{
    if (si->num_lambdas > MAX_LAMBDAS) {
	return 0;
    }

    /*
    if ((si->type == Photometer && r->inst_type != Single_Channel) ||
	(si->type == Radiometer && r->inst_type != Single_Channel) ||
	(si->type == 
    Multifilter
    Radiometer
    Multifilter32
    */

    return 1;
}






/*
** cosine_correct
**
** apply the cosine correction 'si' to the current record
*/
void cosine_correct(RSR_file *r, solarinfo *si, double *data)
{
    double	az, el, corr, g, b, d, cz;
    int		i, j, bi, di, gi;

    if (si == NULL)
	return;

    call_sunae(r);

    if ((el = aep.el) < 0.001)		/* pretty darn low */
	return;

    cz = cos(M_PI_2 - el);

    el *= M_RTOD;
    az = M_RTOD*aep.az;
    if (az < 0.0)
	az += 360.0;
    if (az >= 360.0)
	az -= 360.0;

    for (i=0; i<si->num_lambdas; i++) {
	if (si->lambdas[i].corr_table == NULL)
	    continue;

	bi = si->lambdas[i].dir_ind;
	if (data[bi] < 0.0001) {
#ifdef DEBUG
	    fprintf(stderr, " n/c\n");
#endif
	    continue;
	}

	if ((corr = cos_correction(si, az, el, i)) == -1.0) {
	    tu_msg("impossible switch case in cosine_correct");
	    exit(1);
	}

	if (corr == 1.0)
	    continue;

	di = si->lambdas[i].diff_ind;
	gi = si->lambdas[i].glo_ind;

	b = data[bi]/corr;
	d = data[di];
	g = d + b*cz;

#ifdef DEBUG
	fprintf(stderr, " g/g=%.1f/%.0f", data[gi], g);
	fprintf(stderr, " b/b=%.1f/%.0f", data[bi], b);
	fputs("\n", stderr);
#endif

	data[bi] = b;
	data[gi] = g;
    }
}






double *copy_data(RSR_file *rf)
{
    static double	*v = NULL;
    int			i;

    if (v == NULL)
	if ((v = malloc(rf->rec_n_day * sizeof(double))) == NULL) {
	    sprintf(buf, "Error: copy_data: can't alloc %d doubles",
				rf->record->n_data);
	    tu_msg(buf);
	    return NULL;
	}

    for (i=0; i<rf->record->n_data; i++)
	v[i] = rf->record->data[i];

    return v;
}
