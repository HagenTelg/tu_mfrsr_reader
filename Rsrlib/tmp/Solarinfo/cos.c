#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tokens.h"
#include "cosdefs.h"
#include "tu.h"

extern long Line_Count;

solarinfo	*si;


#ifdef TEST
int main(int argc, char **argv)
{
    int		i;
    void	init_si(solarinfo *),
		print_si(solarinfo *),
		clear_si(solarinfo *),
		fixup_tables(solarinfo *, double);
    /* solarinfo	*read_si(char *, double); */

    for (i=1; i<argc; i++) {
	fprintf(stderr, "%s...", argv[i]);
	if ((si = read_si(argv[i], 42.7)) != NULL) {
	    fixup_tables(si, 42.7);
	    fputs("fine", stderr);
	    clear_si(si);
	    free(si);
	} else    
	    fputs("bad", stderr);
    }
}
#endif


static const char	*parse_err[] = {
	"No parse error",				/* 0 */
	"yacc failed",					/* 1 */
	"header param count wrong",			/* 2 */
	"header param count wrong",			/* 3 */
	"multiple key defs",				/* 4 */
	"alloc failed lam 0",				/* 5 */
	"realloc fail lam > 0",				/* 6 */
	"no key def",					/* 7 */
	"we index out of range (too many tables)",	/* 8 */
	"we table alloc fail",				/* 9 */
	"can't alloc corr_table (we)",			/* 10 */
	"sn index out of range (too many tables)",	/* 11 */
	"can't alloc corr_table (sn)",			/* 12 */
	"sn table alloc fail",				/* 13 */
	"found e-w table but not n-s",			/* 14 */
	"found n-s table, but not e-w",			/* 15 */
	"too many entries in we table",			/* 16 */
	"too many entries in sn table",			/* 17 */
	"not 181 entries in sn table",			/* 18 */
	"not 181 entries in we table",			/* 19 */
	"no tables found for channel",			/* 20 */
	"w-e table found for channel not requesting one", /* 21 */
	"found extra w-e table for this channel",	/* 22 */
	"s-n table found for channel not requesting one", /* 23 */
	"found extra s-n table for this channel",	/* 24 */
	"need at least as many tables as filters",	/* 25 */
	"unknown table mismatch in solarinfo",		/* 26 */
	""};						/* 27 parser produced err msg */



static char	ebuf[512];

solarinfo *read_si(char *fnm, double lat)
{
    extern FILE	*yyin;
    int		parse_code, yyparse(void);
    void	init_si(solarinfo *),
		print_si(solarinfo *),
		fixup_tables(solarinfo *, double),
		clear_si(solarinfo *);

    /* yydebug = 1; */

    if ((yyin = fopen(fnm, "r")) == NULL) {
	sprintf(ebuf, "can't open cosine file %s", fnm);
	tu_msg(ebuf);
	return NULL;
    }

    if ((si = malloc(sizeof(solarinfo))) == NULL) {
	sprintf(ebuf, "solarinfo malloc fail");
	tu_msg(ebuf);
	fclose(yyin);
	return NULL;
    }

    init_si(si);
    if ((parse_code = yyparse()) != 0) {
	if(*parse_err[parse_code]) {   /* non-empty message */
	    sprintf(ebuf, "line %ld: %s",
				Line_Count, parse_err[parse_code]);
	    tu_msg(ebuf);
	}
	clear_si(si);
	free(si);
	fclose(yyin);
	return NULL;
    } else {
#ifdef TEST2
	print_si(si);
#endif
	fclose(yyin);
	fixup_tables(si, lat);

	/* if this is a single channel, swap the g and b positions */
	if (0) {
	/* if (si->type == Radiometer || si->type == Photometer) { */
	    int		i, tmp;

	    for (i=0; i<si->num_lambdas; i++) {
		if (si->lambdas[i].corr_table == NULL)
		    continue;

		tmp = si->lambdas[i].dir_ind;
		si->lambdas[i].dir_ind = si->lambdas[i].glo_ind;
		si->lambdas[i].glo_ind = tmp;
	    }
	}

	return si;
    }
}




void init_si(solarinfo *si)
{
    si->type = No_inst;
    si->num_filt = 0;
    si->num_chan = 0;
    si->unit_id = 0;
    si->key_ind = 0;
    si->num_lambdas = 0;
    si->valid_from = 0.0;
    si->valid_to = 0.0;
    si->lambdas = NULL;
}


void print_si(solarinfo *si)
{
    static const char	*inst_name[] = {"No_inst", "Photometer", "Multifilter",
				"Radiometer", "Multifilter32"};
    int			i, j;

    printf("Instrument is %s\n", inst_name[si->type]);
    printf("%d filters\n", si->num_filt);
    printf("%d columns in output\n", si->num_chan);
    printf("unit id is 0x%04x\n", si->unit_id);
    if (si->type == Multifilter32)
	printf("head id is 0x%08x", si->head_id);
    printf("key wavelength %d ('%s')\n", si->key_ind,
					si->lambdas[si->key_ind].name);
    printf("%d lambdas\n", si->num_lambdas);
    printf("Valid from %f", si->valid_from);
    if (si->valid_to > si->valid_from)
	printf(" to %f", si->valid_to);
    puts("");

    for (i=0; i<si->num_lambdas; i++) {
	printf("Lambda %d: Name=%s; g=%d, b=%d, d=%d; 0x%x\n", i+1,
			si->lambdas[i].name, si->lambdas[i].glo_ind,
			si->lambdas[i].dir_ind, si->lambdas[i].diff_ind,
			si->lambdas[i].corr_kind);
    }

    printf("    ");
    for (j=0; j<si->num_lambdas; j++)
	if (si->lambdas[j].corr_table != NULL)
	    printf("  WE%d  SN%d", j+1, j+1);
    puts("");

    for (j=0; j<181; j++) {
	printf("%3d:", j+1);
	for (i=0; i<si->num_lambdas; i++)
	    if (si->lambdas[i].corr_table != NULL)
		printf(" %4.2f %4.2f", si->lambdas[i].corr_table->we[j],
					si->lambdas[i].corr_table->sn[j]);
	puts("");
    }
}


void clear_si(solarinfo *si)
{
    if (si->lambdas != NULL) {
	int	i;

	for (i=0; i<si->num_lambdas; i++) {
	    free(si->lambdas[i].name);
	    if (si->lambdas[i].corr_table != NULL) {
		free(si->lambdas[i].corr_table->sn);
		free(si->lambdas[i].corr_table->we);
		free(si->lambdas[i].corr_table);
	    }
	}

	free(si->lambdas);
    }
}



/*
** fixup_tables
**
** The Tables read from the solarinfo files are kinda hard to use;
** it would be nice if there were 4 90-point vectors in the 4
** azimuthal directions that listed the cosine correction for each
** degree from the horizon to the zenith, say. But there aren't.
** This function changes the 181-element west-to-east and south-to-
** north vectors around so they are a bit easier to work with...
**
** West/                           East/
** South Horizon             ZenithNorth Horizon           Zenith
** 0000   ....                8888899999                      778
** 0123   ....                5678901234                      890
**
** That is, the west/east vector goes from the western horizon to zenith
** in 1-degree increments as the west-east index goes from 0 to 89. Then
** it goes from the eastern horizon to the zenith as the vector index
** ranges from 90 to 179. The same goes for the south-to-north vector.
** I.e., this function reverses the order of the last 90 elements of the
** we and sn arrays.
**
** But wait, there's more. The tables are created using nomenclature
** appropriate for the northern hemisphere. When the site in question
** is in the southern hemisphere, then north and south are reversed,
** as are east and west. So, reverse the arrays if the site is
** south of the equator.
*/
static void fixup_tables(solarinfo *si, double lat)
{
    int		i, j;
    void	swap(double *, double *);

    for (j=0; j<si->num_lambdas; j++)
	if (si->lambdas[j].corr_table != NULL)
	    for (i=90; i<135; i++) {
		swap(si->lambdas[j].corr_table->we + i,
		     si->lambdas[j].corr_table->we + 270-i);
		swap(si->lambdas[j].corr_table->sn + i,
		     si->lambdas[j].corr_table->sn + 270-i);
	    }

    if (lat < 0.0)
	for (j=0; j<si->num_lambdas; j++)
	    if (si->lambdas[j].corr_table != NULL)
		for (i=0; i<90; i++) {
		    swap(si->lambdas[j].corr_table->we + i,
			 si->lambdas[j].corr_table->we + 90);
		    swap(si->lambdas[j].corr_table->sn + i,
			 si->lambdas[j].corr_table->sn + 90);
		}
}



void swap(double *x, double *y)
{
    double	tmp;

    tmp = *y;
    *y = *x;
    *x = tmp;
}




typedef struct {
    double	ewt1, ewt2, awt1, awt2;
    int		el_deg, quad;
} weight_vector;


/*
** Return a number that may be multiplied by an observation to achieve
** a cosine-corrected value for that observation.
**
** 5/1/95 - this function is called repeatedly with the same azimuth
** and elevation; cache the weights and vectors so those decisions
** aren't repeated.
*/
double cos_correction(solarinfo *si, double az_d, double el_d, int channel)
{
    double	c1, c2, corr, *p1, *p2;
    int		i;
    void	get_vectors(double, double, weight_vector *);
    static weight_vector
		wt;
#ifdef DEBUG
    char p1l[5]="nesw", p2l[5]="eswn";
#endif

    if (si->lambdas[channel].corr_table == NULL)
	return 1.0;

    if (el_d > 89.5)
	return 1.0;

    get_vectors(az_d, el_d, &wt);

    switch (wt.quad) {
    case 0:
	p1 = si->lambdas[channel].corr_table->sn + 90;  /* north */
	p2 = si->lambdas[channel].corr_table->we + 90;  /* east */
	break;
    case 1:
	p1 = si->lambdas[channel].corr_table->we + 90;  /* east */
	p2 = si->lambdas[channel].corr_table->sn;        /* south */
	break;
    case 2:
	p1 = si->lambdas[channel].corr_table->sn;        /* south */
	p2 = si->lambdas[channel].corr_table->we;        /* west */
	break;
    case 3:
	p1 = si->lambdas[channel].corr_table->we;        /* west */
	p2 = si->lambdas[channel].corr_table->sn + 90;  /* north */
	break;
    default:
	return -1.0;
    }

    c1 = p1[wt.el_deg]*wt.ewt1 + p1[wt.el_deg + 1]*wt.ewt2;
    c2 = p2[wt.el_deg]*wt.ewt1 + p2[wt.el_deg + 1]*wt.ewt2;
    corr = c1*wt.awt1 + c2*wt.awt2;

#ifdef DEBUG
fprintf(stderr,"channel=%d el=%.4f az=%.4f",channel,el_d,az_d);
fprintf(stderr," quad=%d(%c|%c) awt1=%.4f ewt1=%.4f",
		wt.quad,p1l[wt.quad],p2l[wt.quad],
		wt.awt1,wt.ewt1);
fprintf(stderr, " p1[%d]=%.4f p1[%d]=%.4f p2[%d]=%.4f p2[%d]=%.4f",
		wt.el_deg, p1[wt.el_deg], wt.el_deg+1, p1[wt.el_deg+1],
		wt.el_deg, p2[wt.el_deg], wt.el_deg+1, p2[wt.el_deg+1]);
fprintf(stderr," corr=%.4f",corr);
#endif

    return corr;
}




/*
** point the correction pointers at the closest 2 directions;
** if the sun is a bit west of south, the closest vector is
** south, and the next is west. Etc.
*/
void get_vectors(double az, double el, weight_vector *wt)
{
    static double	last_az, last_el;
    double		tmp;

    if (last_az == az && last_el == el)
	return;

    last_az = az;
    last_el = el;

    wt->awt2 = modf(az/90.0, &tmp);	/* weight of 2nd direction */
    wt->quad = (int) (tmp + 0.5);	/* quadrant of 1st direction */
    wt->awt1 = 1.0 - wt->awt2;		/* weight of 1st direction */

    wt->ewt2 = modf(el, &tmp);		/* weight of next highest degree */
    wt->el_deg = (int) (tmp + 0.5);	/* this degree */
    wt->ewt1 = 1.0 - wt->ewt2;		/* weight of this degree */
}
