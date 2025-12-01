/*
** cosdefs.h
**
** typedefs and stuff for cosine correction
**
** this created either automagically by p2c or by hand by jas;
**
** 12/20/94 rs - reformatted to my taste
*/

#ifndef COSDEFS_H
#define COSDEFS_H

typedef enum {
    None = 0, Table = 1, Tablenl = 2, Nolang = 4,
    Licor = 8, Sirad = 16, Void = 32, Key = 64
} correction;


typedef enum {
    No_inst = 0, Photometer, Multifilter, Radiometer, Multifilter32
} instrument;


typedef struct {
    double	*sn,    /* array of south-north responses */
		*we;    /* array of west-east responses */
} response;


typedef struct {
    char	*name;		/* measurement name */
    int 	glo_ind,	/* column index for global */
		dir_ind,	/* column index for direct */
		diff_ind;	/* column index for diffuse */
    correction	corr_kind;	/* type of correction/langley analysis */
    response	*corr_table;	/* actual cosine response table, if reqd.  */
} lambda;


typedef struct {
    instrument 	type;		/* what kind of instrument is this? */
    int  	num_filt,	/* number of RSR filters */
		num_chan,	/* number of columns in the unpacked output */
		unit_id,	/* unit number */
		head_id,	/* head id */
		key_ind,	/* key wavelength index number, -1 if none */
		num_lambdas;	/* # between LAMBDAS and END in solarinfo */
    double	valid_from,	/* starting date this solarinfo is valid */
		valid_to;	/* ending date */
    lambda	*lambdas;	/* pointer to lambda data structures */
} solarinfo;


solarinfo	*read_si(char *, double lat);
double		cos_correction(solarinfo *, double, double, int),
		my_strtod(char *, char **);

#define MAX_LAMBDAS     64      /* maximum number of correction tables */

#endif
