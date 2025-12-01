#ifndef RSR_LIB
#define RSR_LIB

/*
** This is all perfectly self-explanatory.
**
** rs 10/5/94
*/

#include <stdio.h>
#include <time.h>

#define TYPE1_HEADER_SIZE	27
#define TYPE2_HEADER_SIZE	35

typedef signed short int	RSR_datum;


typedef struct {
    RSR_datum	*data;
    time_t	obs_time;
    int		is_gap;
    short	n_data;
} RSR_rec;


typedef enum {
    Unknown_Instrument = 0,
    Single_Channel,
    Multi_Filter_16,
    Multi_Filter_32,
    Type2
} RSR_type;

/*
** raw header types raw_type_1 and raw_type_2
*/
typedef struct { 
    unsigned char   sr,		/* Soft. Rev. one of 10,11,12,77 or 78 */
                    id[2],      /* Serial number */
                    lng[2],     /* 16 bit fraction of a circle */
                    lat[2],
                    flags,
                    auxs[2],                       
                    bps[2],
                    rate[2],    /* sampling rate in seconds */
                    ave[2],     /* averaging period in seconds */
                    dbsec[3],   /* seconds of the day for first record */
                    dbdate[2],  /* days since 1/1/1900 of first record */
                    photocal[2], /* gain coeff. for photodetector */
                    photoff[2], /* offset to photodetector OR high byte = */
                                /* number of active channels */
                    bytes[2];   /* number of signif. bytes in last record */
} raw_type_1;
 
typedef struct {
    unsigned char   sr,         /* Soft. Rev. - 13 for type II headers */
                    id[2],      /* Conventional ID number */
                    hid[2],     /* Head ID number */
                    lng[2],     /* 16 bit fraction of a circle */
                    lat[2],
                    flags,      /* */
                    rate[2],    /* sampling rate in seconds */
                    ave[2],     /* averaging period in seconds */
                    dbdate[2],  /* days since 1/1/1900 of first record */
                    dbsec[3],   /* seconds of the day for first record */
                    ndiodes,
                    dauxs[4],   /* */
                    auxs[4],
                    cntrs[3],
                    bytes[3],   /* number of signif. bytes in last record */
                    errs;
} raw_type_2;


typedef struct {
    double		latitude, longitude;
    char		soft_rev, flags, extra[16], bipolar[16],
			daytime[32], all_the_time[32], counter[6],
			immed_out, low_power, band_on, met_on, dummy,
			volt_dog, oao, halt, err;
    unsigned short	raw_extra, raw_bipolar;
    unsigned long	days_1900, secs_today,
			raw_daytime, raw_all_the_time, raw_counter;
    unsigned int	sample_rate, avg_period,
			unit_id, head_id;
    signed int		rsr_gain, rsr_offset,
			diodes;
} RSR_header;

typedef struct {
    FILE		*f;
    RSR_type		inst_type;
    unsigned int	header_length,
			rec_len_day, rec_n_day,
			rec_len_nite, rec_n_nite;
    unsigned long	data_length,
			curr_rec_no;
    char		*filename, *polar_day, *polar_nite,
			is_authentic,
			skip_unpack,
			headerOK,
			deleteOnError;
    time_t		start_time;
    RSR_header		*head;
    void		*raw_head;
    RSR_rec		*record;
    /* RSR_datum	*record; */
/* #ifdef __MSDOS__ */
/*    unsigned char far	*data, *cdp;  */ /* current data pointer */
/* #else */
    unsigned char	*data, *cdp;  /* current data pointer */
/* #endif */
} RSR_file;


/*
** User-visible functions
*/

RSR_file	*rsr_open_file(const char *);
RSR_rec		*rsr_next_record(RSR_file *);
void		rsr_file_delete(RSR_file *),
		rsr_rec_delete(RSR_rec *),
		rsr_print_header(RSR_file *, char *, int);
char		*rsr_last_message(void);

char 		*rsr_strftime(double jday, char *buf, int bufsize, char *fmt),
		pd_strftime(char *, size_t, const char *, const struct tm *);
time_t 		rsr_j2unix(double jdays);
double 		rsr_unix2j(time_t secs);
int 		rsr_write_header(RSR_file *rf, FILE *fp);
int 		rsr_write_records(RSR_file *rf, long from, long to, FILE *fp);
int 		rsr_write_signature(char *sig, FILE *fp);


/*
** functions called by library routines
*/
void		rsr_err(char *),
		rsr_clear_err(void),
		rsr_set_current_time(RSR_file *);
int		sstoa(signed short, char *),
		fmt4(double, char *);
char		*rsr_strdup(const char *);


#endif	/* RSR_LIB */
