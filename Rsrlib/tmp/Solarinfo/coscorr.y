%token TOK_COMM TOK_CR TOK_WS TOK_LF TOK_INT TOK_FLOAT TOK_END
%token TOK_KEY TOK_LAMBDAS TOK_VAR TOK_EOF TOK_CRLF TOK_UNK
%token TOK_SN TOK_WE TOK_MULTI TOK_NONE TOK_TABLE TOK_TABLENL
%token TOK_NOLANG TOK_LICOR TOK_SIRAD TOK_VOID TOK_PHOTO TOK_RADIO
%token TOK_MULTI32 TOK_HEX

%start solarinfo

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** 10/27/95 rs
** couple things in here:
** corr_kind not initialized in "new_channel"; fixed, and
** removed restriction on exactly 1 key channel; can now have 0 or 1.
**
** 4/5/96 rs
** the ids in the header can be hex or decimal.
**
*/

#include "cosdefs.h"

extern solarinfo	*si;
extern char		*yytext;

extern long		Line_Count;

static int		ret, hed_ind = 0, lam_ind = 0, sn_i, we_i,
			sn_ind, we_ind,		/* current table */
			*table_status=NULL;
			/* 0=none; 1=needs; 2=found we; 4=found sn */
static double		hed[16];
static char		err[256];

int			setup_header(void),
			setup_lambdas(void),
			new_channel(void),
			setup_sn_table(void),
			setup_we_table(void),
			new_sn_number(void),
			new_we_number(void),
			finish_tables(void);
%}

%%

solarinfo:	header { if ((ret = setup_header()) != 0) return ret; }
		lambdas { if ((ret = setup_lambdas()) != 0) return ret; }
		tables { if ((ret = finish_tables()) != 0) return ret; }
		;

header:		rsr_type header_info
		;

rsr_type:	TOK_MULTI { si->type = Multifilter; } |
		TOK_PHOTO { si->type = Photometer; } |
		TOK_RADIO { si->type = Radiometer; } |
		TOK_MULTI32 { si->type = Multifilter32; } |
		TOK_VAR {
			sprintf(err,
			  "line %ld: unrecognizeable RSR type: '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27;
		}
		;

header_info:	number { hed[hed_ind++] = my_strtod(yytext, NULL); } |
		header_info number { hed[hed_ind++] = my_strtod(yytext, NULL); }
		;

number:		TOK_INT |
		TOK_FLOAT |
		TOK_HEX
		;

lambdas:	TOK_VAR { if (!check_var("lambdas")) {
			sprintf(err,
			  "line %ld: expecting LAMBDAS, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		}}
		channels 
		ending
		;

ending: 	TOK_END | 
		/* anything but END is unacceptable */
		{ 
			sprintf(err,
			  "line %ld: expecting END, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		}
		;

channels:	channel |
		channels channel
		;

channel:	TOK_VAR { if ((ret = new_channel()) != 0) return ret; }
		cols3 cor_types {
		    lam_ind++;
		}
		;

cols3:		TOK_INT { si->lambdas[lam_ind].glo_ind = atoi(yytext); }
		TOK_INT { si->lambdas[lam_ind].diff_ind = atoi(yytext); }
		TOK_INT { si->lambdas[lam_ind].dir_ind = atoi(yytext); }
		;

cor_types:	cor_type |
		cor_types cor_type
		;

cor_type:	TOK_NONE { si->lambdas[lam_ind].corr_kind += None; } |
		TOK_NOLANG { si->lambdas[lam_ind].corr_kind += Nolang; } |
		TOK_LICOR { si->lambdas[lam_ind].corr_kind += Licor; } |
		TOK_SIRAD { si->lambdas[lam_ind].corr_kind += Sirad; } |
		TOK_TABLE {
		    si->lambdas[lam_ind].corr_kind += Table;
		    table_status[lam_ind] = 1;	/* needs a table */
		} |
		TOK_TABLENL {
		    si->lambdas[lam_ind].corr_kind += Tablenl;
		    table_status[lam_ind] = 1;	/* needs a table */
		} |
		TOK_VOID {
		    si->lambdas[lam_ind].corr_kind += Void;
		    table_status[lam_ind] = 1;	/* needs a table */
		} |
		TOK_KEY { si->lambdas[lam_ind].corr_kind += Key; }
		;

tables:		table |
		tables table
		;

table:		table_sn | 
		table_we |
		error { 
			sprintf(err,
			  "line %ld: expecting table label, got '%s'", Line_Count, yytext);
			tu_msg(err);
			return 27; 
		}
		;

table_sn:	TOK_SN { if ((ret = setup_sn_table()) != 0) return ret; }
		sn_numbers ending {
		    if (sn_i != 181)
			return 18;	/* not 181 entries in sn table */
		} 
		;

table_we:	TOK_WE { if ((ret = setup_we_table()) != 0) return ret; }
		we_numbers ending {
		    if (we_i != 181)
			return 19;	/* not 181 entries in we table */
		}
		;
		
sn_numbers:	number { if ((ret = new_sn_number()) != 0) return ret; } |
		sn_numbers number { if ((ret=new_sn_number())!=0) return ret; }
		;

we_numbers:	number { if ((ret = new_we_number()) != 0) return ret; } |
		we_numbers number { if ((ret=new_we_number())!=0) return ret; }
		;


%%

int setup_header(void)
{
    if (si->type == Multifilter32) {
	if (hed_ind != 5 && hed_ind != 6)
	    return 2;	/* header param count wrong */
	si->num_filt = hed[0];
	si->num_chan = hed[1];
	si->unit_id = hed[2];
	si->head_id = hed[3];
	si->valid_from = hed[4];
	si->valid_to = hed_ind == 6 ? hed[5] : si->valid_from - 1.0;
    } else {
	if (hed_ind != 4 && hed_ind != 5)
	    return 3;	/* header param count wrong */
	si->num_filt = hed[0];
	si->num_chan = hed[1];
	si->unit_id = hed[2];
	si->valid_from = hed[3];
	si->valid_to = hed_ind == 5 ? hed[4] : si->valid_from - 1.0;
    }

    return 0;
}



int setup_lambdas(void)
{
    int		i;

    si->num_lambdas = lam_ind;

    /* locate the 'key channel' */
    si->key_ind = -1;
    for (i=0; i<lam_ind; i++) {
	if (si->lambdas[i].corr_kind & Key) {
	    if (si->key_ind != -1)
		return 4;	/* multiple key defs */
	    si->key_ind = i;
	}
    }

/*
** 10/27/95
** Jim says that a key channel is not a necessity...
**
*/
    /*
    if (si->key_ind == -1)
	return 7; */	/* no key def */

    return 0;
}



int new_channel(void)
{
    if (lam_ind == 0) {
	if ((si->lambdas = malloc(sizeof(lambda))) == NULL)
	    return 5;		/* alloc failed lam 0 */
	if ((table_status = malloc(sizeof(int))) == NULL)
	    return 5;
    } else {
	if ((si->lambdas = realloc(si->lambdas,
			(lam_ind+1)*sizeof(lambda))) == NULL)
	    return 6;		/* realloc fail lam > 0 */
	if ((table_status=realloc(table_status,(lam_ind+1)*sizeof(int)))==NULL)
	    return 6;
    }

    si->lambdas[lam_ind].name = strdup(yytext);
    si->lambdas[lam_ind].corr_table = NULL;
    si->lambdas[lam_ind].corr_kind = None;
    table_status[lam_ind] = 0;		/* don't know yet */

    return 0;
}



/*
** for both setup_we_table and setup_sn_table
** 
** The 'lambdas' section of the file has already told us which of
** the num_lambdas lambdas require a table. At this point, the data
** structure slot for the current (about to read) table is unallocated; 
** first make sure that we expect a table for 'this' slot; then
** allocate the table and it's cosine vectors. Finally, indicate, via
** the table_status vector, that we've found a table we've been looking for.
*/
int setup_we_table(void)
{
    sscanf(yytext, "%*2c%d", &we_ind);

    if (we_ind < 1 || we_ind > si->num_lambdas)
	return 8;	/* we index out of range */
    we_ind--;	/* count from 0... */

    /* are we looking for this table? */
    if (!(table_status[we_ind] & 1))
	return 21;	/* found a table for a lambda that doesn't need 1 */
    if (table_status[we_ind] & 2)
	return 22;	/* too many we tables for this channel */

    if (!(table_status[we_ind] & 6)) {		/* make a new corr_table */
	if ((si->lambdas[we_ind].corr_table=malloc(sizeof(response))) == NULL)
	    return 10;	/* can't alloc corr_table */
	si->lambdas[we_ind].corr_table->sn = NULL;
    }
    if ((si->lambdas[we_ind].corr_table->we =
			malloc(181*sizeof(double))) == NULL)
	return 9;	/* we table alloc fail */

    we_i = 0;
    table_status[we_ind] += 2;		/* cool */

    return 0;
}




int setup_sn_table(void)
{
    sscanf(yytext, "%*2c%d", &sn_ind);

    if (sn_ind < 1 || sn_ind > si->num_lambdas)
	return 11;	/* sn index out of range */
    sn_ind--;

    /* is this table required? */
    if (!(table_status[sn_ind] & 1))
	return 23;	/* sn table for channel not requesting one */
    if (table_status[sn_ind] & 4)
	return 24;	/* too many s-n table for this channel */

    if (!(table_status[sn_ind] & 6)) {		/* make a new corr_table */
	if ((si->lambdas[sn_ind].corr_table = malloc(sizeof(response))) == NULL)
	    return 12;	/* can't alloc corr table */
	si->lambdas[sn_ind].corr_table->we = NULL;
    }
    if ((si->lambdas[sn_ind].corr_table->sn =
			malloc(181*sizeof(double))) == NULL)
	return 13;	/* sn table alloc fail */

    sn_i = 0;
    table_status[sn_ind] += 4;

    return 0;
}




int new_we_number(void)
{
    if (we_i > 180)
	return 16;	/* too many entries in we table */

    si->lambdas[we_ind].corr_table->we[we_i] = my_strtod(yytext, NULL);

    we_i++;

    return 0;
}




int new_sn_number(void)
{
    if (sn_i > 180)
	return 17;	/* too many entries in sn table */

    si->lambdas[sn_ind].corr_table->sn[sn_i] = my_strtod(yytext, NULL);

    sn_i++;

    return 0;
}





int finish_tables(void)
{
    int		i, tables = 0;

    for (i=0; i<si->num_lambdas; i++)
	switch (table_status[i]) {
	case 7:
	    tables++;
	    /* fallthrough */
	case 0:		/* no table requsted */
	    break;
	case 1:
	    return 20;	/* no tables for requested channel */
	case 3:
	    return 14;	/* found we but not sn */
	case 5:
	    return 15;	/* found sn but not we */
	default:	/* "impossible" */
	    return 26;	/* unknown table mismatch */
	}

    if (tables < si->num_filt)
	return 25;	/* fewer tables than filters */

    if (table_status != NULL)
	free(table_status);

    return 0;
}

int check_var(char *str)
{
    char *p, *q;

    p = str;
    q = yytext;

    while(*p) {
	if( ! (*p == *q || (*p-32) == *q || (*q-32) == *p) )
		return 0;
	p++;q++;
    }

    return 1;
}

