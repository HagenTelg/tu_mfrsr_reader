#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#ifndef __MSDOS__
#include <unistd.h>	/* SEEK_SET et al */
#else
#include <alloc.h>	/* farmalloc, farfree */
#endif
#ifdef sparc
#include <stdarg.h>	/* for sprintf fix */
#endif

#include "rsrlib.h"

#define ERR_NOERROR   0x00
#define ERR_SHORTFILE 0x01

static char	rsr_errbuf[512];
static char	*my_strerror(int);
char ErrCode = ERR_NOERROR;

/*
#ifdef __MSDOS__
#define	DOS_BIG_BUF	32768UL
#endif
*/



/**
*** Library for rsr files.
***
*** 8/2/95 rob
*** Error negating type 2 datum. Fixed.
**
***
*** 3/12/96 rob
*** In rsr_open_file, polar_day and polar_nite are unitialized. Found
*** by Jim when it dumped core on a solaris machine.
***
*** 4/5/96 rob
*** When the 'data record' is allocated, the variable that indicates
*** how many fields are in the current record is uninitialized - because
*** there IS no current record yet. Jim
*** was testing this before it got set, and got a core dump or something.
*** So, we initialize it to -2; but the real solution is not to examine
*** the variable until you've read something.
***
*** 11/7/96 rob
*** In type 2 files, there is the "error byte" which indicates that there
*** was an operational error in the shadowband sometime since the last
*** download. Until now, I had been checking this and refusing to
*** continue if the byte was nonzero. However, the thinking is that a
*** nonzero error byte is not necessarily cause for complete failure.
*** The data should supposedly still be usable in most cases. So, the
*** byte is checked and if set is noted, but the program is allowed to
*** to continue, i.e., it in itself is not cause for the call to
*** rsr_open_file to fail.
***
*** 2/6/96 jim
*** Changed rsr_write_header.
***
*** 8/6/98 jim
*** Jim Barnard asked that rsrsplit, ph, etc. try to make due with what they
*** have in the event that rsr_open_file() detects that bytes_left bytes aren't
*** available in the file (i.e., it's truncated).  ARM machinary is apparently
*** responsible for loping off 100 bytes or records or whatever from MFRSR dump
*** files.  To get around this error -- which formerly would result in a call
*** to rsr_delete() and which would subsequently result in the deletion of all header
*** and record info -- I've added a field to the RSR_file struct called deleteOnError.
*** If the rf->deleteOnError isn't set, then the memory alloc()'d for *rf is
*** left intact.
***
*** 8/11/98 jim
*** made it so rsrlibc doesn't automatically give up the ghost when it encounters
*** a bad record length.  This enables programs that use the library to carry on
*** using whatever they can get from the data file.  This is probably more sensible
*** than throwing it all away because the file was corrupted toward the end.
**/






/*
** rsr_open_file
** Try to open the file 'fnm'. If it is a valid RSR file,
** set up all the quantities necessary to access data records
** and return a pointer to a new RSR_file object. If there
** is any problem, return NULL.
*/
#ifdef INDEX_FILE
RSR_file *rsr_open_file(const char *fnm, int do_indexing)
#else
RSR_file *rsr_open_file(const char *fnm)
#endif
{
    int			rsr_unpack_header(RSR_file *, unsigned char *),
			rsr_index_file(RSR_file *),
			rsr_setup_data(RSR_file *);
    char		*rsr_strdup(const char *);
    RSR_file		*rf;
    unsigned char	c, *hb;

    /* no errors yet! */
    rsr_clear_err();

    /* get space for it */
    if ((rf = malloc(sizeof(RSR_file))) == NULL) {
	rsr_err("Can't allocate memory for an RSR_file");
	return NULL;
    }

    rf->head = NULL;
    rf->headerOK = 0;
    rf->raw_head = NULL;
    rf->data = NULL;
    rf->filename = NULL;
    rf->record = NULL;
    rf->polar_day = NULL;
    rf->polar_nite = NULL;

    if ((rf->filename = rsr_strdup(fnm)) == NULL) {
	rsr_file_delete(rf);
	return NULL;
    }

    /* try to open the file */
    if ((rf->f = fopen(rf->filename, "rb")) == NULL) {
	sprintf(rsr_errbuf, "Can't open file '%s': %s",
		    rf->filename, my_strerror(errno));
	rsr_err(rsr_errbuf);
	rsr_file_delete(rf);
	return NULL;
    }

    /* check the file type */
    rf->inst_type = Unknown_Instrument;
    if (fread(&c, 1, 1, rf->f) != 1) {
	sprintf(rsr_errbuf, "Can't read byte 1 from '%s'", fnm);
	rsr_err(rsr_errbuf);
	rsr_file_delete(rf);
	return NULL;
    }

    switch (c) {
    case 10:
    case 11:
	rf->inst_type = Multi_Filter_16;
	rf->header_length = TYPE1_HEADER_SIZE;
	break;
    case 12:
	rf->inst_type = Multi_Filter_32;
	rf->header_length = TYPE1_HEADER_SIZE;
	break;
    case 13:
	rf->inst_type = Type2;
	rf->header_length = TYPE2_HEADER_SIZE;
	break;
    case 77:
    case 78:
	rf->inst_type = Single_Channel;
	rf->header_length = TYPE1_HEADER_SIZE;
	break;
    default:
	sprintf(rsr_errbuf, "File '%s' isn't rsr: byte 1 is %d", fnm, c);
	rsr_err(rsr_errbuf);
	rsr_file_delete(rf);
	return NULL;
    }

    /* first byte is reasonable; now try to read the rest of the header */
    rewind(rf->f);
    if ((hb = malloc(rf->header_length)) == NULL) {
	rsr_err("Can't allocate memory for an RSR header buffer");
	rsr_file_delete(rf);
	return NULL;
    }
    if (fread(hb, 1, rf->header_length, rf->f) != rf->header_length) {
	rsr_err("Can't read an RSR file status packet");
	rsr_file_delete(rf);
	return NULL;
    }

    /* try to unpack the header ('status packet') */
    if (!rsr_unpack_header(rf, hb)) {
	free(hb);
	rsr_file_delete(rf);
	return NULL;
    }
    rf->raw_head = (void *) hb;
    
    /* the header checks out */
    rf->headerOK = 1;

    /*
    ** set the time of the first record in the file,
    ** and set the current record number to 0.
    */
    rf->start_time = rsr_j2unix(rf->head->days_1900 +
				rf->head->secs_today/86400.0);
    rf->curr_rec_no = 0;
    rf->skip_unpack = 0;

    /* set up the data buffer and pointers */
    while (!rsr_setup_data(rf)) {
	if(ErrCode == ERR_SHORTFILE) {
		/* file is an authentic file but is truncated.  We procede by "fooling"
		   rsrlib into thinking it's now an unauthentic file, thereby requesting
		   that rsr_setup_data() figure out how long it is.  */
		rf->data_length = 0UL;
	}
	else {
		rsr_file_delete(rf);
		return rf;	/* since the header's OK, we return rf in the event that deleteOnError is 0 */
	}
    }

#ifdef INDEX_FILE
    if (do_indexing)
	if (!rsr_index_file(rf))
	    rsr_err("Can't index file...random access not available.");
#endif

    return rf;
}





#ifdef INDEX_FILE
/*
** create the bit vector of record indecies. if unable to for some reason,
** like memory short, then return 0. Else return 1.
**
** this routine creates the rf->index_map bit vector; it also sets the
** rf->recs_in_file variable.
*/
int rsr_index_file(RSR_file *rf)
{
    return 0;
}
#endif	/* INDEX_FILE */






/*
** rsr_strdup
** not all runtimes have strdup
*/
char *rsr_strdup(const char *s)
{
    char	*p;

    if ((p = malloc(strlen(s) + 1)) == NULL) {
	rsr_err("Can't get memory to duplicate a string");
	return NULL;
    }

    strcpy(p, s);

    return p;
}





/*
** rsr_unpack_header
*/
int rsr_unpack_header(RSR_file *rf, unsigned char *buf)
{
    int		rsr_check_header(RSR_file *),
		rsr_unhead_1(RSR_file *, unsigned char *),
		rsr_unhead_2(RSR_file *, unsigned char *),
		(*head_unpacker)(RSR_file *, unsigned char *);

    /* first see if we can get memory for the header data structure */
    if ((rf->head = malloc(sizeof(RSR_header))) == NULL) {
	rsr_err("Can't alloc memory for a RSR_header in rsr_unpack_header");
	return 0;
    }

    /* now do the unpack depending on if it's type 2 or not */
    if (rf->inst_type == Type2)
	head_unpacker = rsr_unhead_2;
    else
	head_unpacker = rsr_unhead_1;

    rf->head->diodes = 0;		/* just make sure */
    if (!head_unpacker(rf, buf))
	return 0;

    return rsr_check_header(rf);
}



/*
** rsr_unhead_1
** unpack a type 1 header
*/
int rsr_unhead_1(RSR_file *rf, unsigned char *buf)
{
    unsigned short	bp;
    int			i;

    rf->head->soft_rev = buf[0];
    rf->head->unit_id = (((unsigned) buf[1]) << 8) + buf[2];
    rf->head->longitude = 360.0 * (((unsigned) buf[3] << 8) + buf[4]) / 65536.0;
    rf->head->latitude = 360.0 * (((unsigned) buf[5] << 8) + buf[6]) / 65536.0;
    rf->head->flags = buf[7];
    rf->head->immed_out = buf[7] & 0x80 ? 1 : 0;
    rf->head->low_power = buf[7] & 0x40 ? 1 : 0;
    rf->head->band_on = buf[7] & 0x20 ? 1 : 0;
    rf->head->met_on = buf[7] & 0x10 ? 1 : 0;
    rf->head->dummy = buf[7] & 0x08 ? 1 : 0;
    rf->head->volt_dog = buf[7] & 0x04 ? 1 : 0;
    rf->head->oao = buf[7] & 0x02 ? 1 : 0;
    rf->head->halt = buf[7] & 0x01 ? 1 : 0;

    rf->head->raw_extra = (((unsigned int) buf[8]) << 8) + buf[9];
    rf->head->raw_bipolar = (((unsigned int) buf[10]) << 8) + buf[11];

    for (i=15, bp=0x80; bp; i--, bp >>= 1)
	rf->head->extra[i] = buf[8] & bp ? 1 : 0;
    for (bp=0x80; bp; i--, bp >>= 1)
	rf->head->extra[i] = buf[9] & bp ? 1 : 0;
    for (i=15, bp=0x80; bp; i--, bp >>= 1)
	rf->head->bipolar[i] = buf[10] & bp ? 1 : 0;
    for (bp=0x80; bp; i--, bp >>= 1)
	rf->head->bipolar[i] = (buf[11] & bp) ? 1 : 0;
    rf->head->sample_rate = (((unsigned) buf[12]) << 8) + buf[13];
    rf->head->avg_period = (((unsigned) buf[14]) << 8) + buf[15];
    rf->head->secs_today = (((unsigned long) buf[16]) << 16) +
				(((unsigned long) buf[17]) << 8) + buf[18];
    rf->head->days_1900 = (((unsigned long) buf[19]) << 8) + buf[20];
    rf->head->rsr_gain = (buf[21] << 8) + buf[22];
    if (rf->inst_type == Single_Channel) {
	rf->head->rsr_offset = (buf[23] << 8) + buf[24];
	rf->head->diodes = 1;	/* sort of...it's really 0 */
    } else {
	rf->head->diodes = buf[23];
	rf->head->rsr_offset = 0; /* sort of...there really isn't one */
    }

    rf->data_length = (((unsigned long) buf[25]) << 8) + buf[26];

    return 1;
}



/*
** rsr_unhead_2
** unpack a type 2 header
*/
int rsr_unhead_2(RSR_file *rf, unsigned char *buf)
{
    unsigned long	tul, bp;
    int			i;

    rf->head->soft_rev = buf[0];
    rf->head->unit_id = (((unsigned) buf[1]) << 8) + buf[2];
    rf->head->head_id = (((unsigned) buf[3]) << 8) + buf[4];
    rf->head->longitude = 360.0 * (((unsigned) buf[5] << 8) + buf[6]) / 65536.0;
    rf->head->latitude = 360.0 * (((unsigned) buf[7] << 8) + buf[8]) / 65536.0;
    rf->head->flags = buf[9];
    rf->head->immed_out = buf[9] & 0x80 ? 1 : 0;
    rf->head->low_power = buf[9] & 0x40 ? 1 : 0;
    rf->head->band_on = buf[9] & 0x20 ? 1 : 0;
    rf->head->met_on = buf[9] & 0x10 ? 1 : 0;
    rf->head->dummy = buf[9] & 0x08 ? 1 : 0;
    rf->head->volt_dog = buf[9] & 0x04 ? 1 : 0;
    rf->head->oao = buf[9] & 0x02 ? 1 : 0;
    rf->head->halt = buf[9] & 0x01 ? 1 : 0;
    rf->head->sample_rate = (((unsigned) buf[10]) << 8) + buf[11];
    rf->head->avg_period = (((unsigned) buf[12]) << 8) + buf[13];
    rf->head->days_1900 = (((unsigned long) buf[14]) << 8) + buf[15];
    rf->head->secs_today = (((unsigned long) buf[16]) << 16) +
				    (((unsigned long) buf[17]) << 8) + buf[18];
    rf->head->diodes = buf[19];

    tul = (((unsigned long) buf[20]) << 24) +
				(((unsigned long) buf[21]) << 16) +
				(((unsigned long) buf[22]) << 8) + buf[23];
    for (i=31, bp=0x80000000L; bp; i--, bp >>= 1)
	rf->head->daytime[i] = tul & bp ? 1 : 0;
    rf->head->raw_daytime = tul;

    tul = (((unsigned long) buf[24]) << 24) +
				(((unsigned long) buf[25]) << 16) +
				(((unsigned long) buf[26]) << 8) + buf[27];
    for (i=31, bp=0x80000000L; bp; i--, bp >>= 1)
	rf->head->all_the_time[i] = tul & bp ? 1 : 0;
    rf->head->raw_all_the_time = tul;

    tul = (((unsigned long) buf[28]) << 16) +
				(((unsigned long) buf[29]) << 8) + buf[30];
    for (i=5, bp=0x00f00000L; bp; i--, bp >>= 4)
	rf->head->counter[i] = tul & bp ? 1 : 0;
    rf->head->raw_counter = tul;

    rf->data_length = (((unsigned long) buf[31]) << 16) +
				(((unsigned long) buf[32]) << 8) + buf[33];
    rf->head->err = buf[34];

    return 1;
}






/*
** rsr_check_header
** do some sanity checks on the RSR_file, return 1 if OK, 0 otherwise
*/
int rsr_check_header(RSR_file *rf)
{
    unsigned long	sec_2;
    int			i, n;

    /* first check things common to all files */
    if (rf->head->latitude < -180.0 || rf->head->latitude > 360.0) {
	rsr_err("Latitude must be -180..360");
	return 0;
    }

    if (rf->head->longitude < -180.0 || rf->head->longitude > 360.0) {
	rsr_err("Longitude must be -180..360");
	return 0;
    }

    if (rf->head->secs_today >= 86402UL) {
	rsr_err("Today's start second must be 0..86400");
	return 0;
    }

    if (rf->head->days_1900 < 32850UL || rf->head->days_1900 > 36500UL) {
	rsr_err("RSR data start day must be in the 1990s");
	return 0;
    }

    sec_2 = 86400UL / 2;
    if (rf->head->sample_rate > sec_2) {
	rsr_err("Sample rate must be 15..43200");
	return 0;
    }
    if (rf->head->avg_period > sec_2) {
	rsr_err("Averaging period must be 15..43200");
	return 0;
    }
    if (rf->head->avg_period % rf->head->sample_rate != 0) {
	rsr_err("Averaging period mod sample rate must be 0");
	return 0;
    }
    if (sec_2 % rf->head->sample_rate != 0) {
	rsr_err("43200 mod sample rate must be 0");
	return 0;
    }
    if (sec_2 % rf->head->avg_period != 0) {
	rsr_err("43200 mod averaging period must be 0");
	return 0;
    }


    /* now do type-specific checks */
    switch (rf->inst_type) {
    case Type2:
	if (rf->head->err) {
	    rsr_err("Type 2 file err bit is set");
	    /* return 0; */
	}
	if (rf->head->diodes < 0 || rf->head->diodes > 7) {
	    rsr_err("Number of diodes must be 0..7");
	    return 0;
	}
	for (i=0, n=0; i<32; i++) {
	    if (rf->head->daytime[i]) n++;
	    if (rf->head->all_the_time[i]) n++;
	}
	for (i=0; i<6; i++)
	    if (rf->head->counter[i]) n++;
	if (n == 0 && !rf->head->band_on) {
	    rsr_err("Type 2 file has no all the time, daytime, nor counters, and band is off");
	    return 0;
	}
	/*
	** should these be checked for something ?
	if (rf->head->unit_id ) {
	}
	if (rf->head->head_id ) {
	}
	*/
	break;

    case Multi_Filter_16:
    case Multi_Filter_32:
	if (rf->head->diodes < 0 || rf->head->diodes > 7) {
	    rsr_err("Number of diodes must be 0..7");
	    return 0;
	}
	/*
	** check this ?
	if (rf->head->unit_id ) {
	}
	*/
	break;

    case Single_Channel:
	/*
	** check this ?
	if (rf->head->unit_id ) {
	}
	*/
	break;

    case Unknown_Instrument:
    default:
	rsr_err("rsr_check_header: Got an unknown instrument (?)"); 
	return 0;
    }

    return 1;
}






/*
** rsr_setup_data
** Since the header has been checked and appears ok, then in all
** likelihood an actual RSR file is in hand. It is possible that
** the file has been postprocessed, which means that the bytecount
** in the file doesn't mean anything, and we have to figure out
** (from the file size) how big the data buffer should be.
** In any case, this function's intent is to prepare to read the
** RSR data section into memory.
** At this writing, the scheme is to allocate the entire data buffer
** in RAM at once, then read all the data into this buffer. At some point,
** it may be necessary to create some buffered input scheme to
** accommodate both extremely large files and machines with limited
** address space, i.e., MS-DOS.
*/
int rsr_setup_data(RSR_file *rf)
{
    int		rsr_rec_lengths(RSR_file *),
		rsr_alloc_rec(RSR_file *);
/*
#ifdef __MSDOS__
    unsigned long	blen;
    size_t		got;
    unsigned char far	*cp;
#endif
*/

    rf->is_authentic = 1;

    /* if the header indicates data len 0, determine it empirically (?) */
    if (rf->data_length == 0UL) {
	long	fpos;

	if (fseek(rf->f, 0L, SEEK_END)) {
	    rsr_err("Unable to seek to end of extended RSR file");
	    return 0;
	}
	if ((fpos = ftell(rf->f)) == -1L) {
	    rsr_err("Unable to determine length of extended RSR file");
	    return 0;
	}
	if (fpos <= rf->header_length) {
	    rsr_err("Extended RSR file has no data part");
	    return 0;
	}
	rf->data_length = fpos - rf->header_length;
	if (fseek(rf->f, rf->header_length, SEEK_SET)) {
	    rsr_err("Unable to seek to end of header in extended RSR file");
	    return 0;
	}
	rf->is_authentic = 0;
  }
    /* now make sure that the data length is sensible */
    if (rf->data_length == 0) {
	rsr_err("No data section in RSR file");
	return 0;
    }

    /*
    ** try to allocate the data buffer
    ** if this is DOS, use farmalloc
    */
/*
#ifdef __MSDOS__
    if ((rf->data = farmalloc(rf->data_length)) == NULL) {
	rsr_err("Insufficient DOS memory for data buffer");
	return 0;
    }
#else
*/
    if ((rf->data = malloc(rf->data_length)) == NULL) {
	rsr_err("Insufficient memory for data buffer");
	return 0;
    }
/* #endif */

    /*
    ** try to read the data section into its buffer
    ** if this is DOS, then we gotta
    ** use fread in a loop until the whole this is read
    */
/* #ifdef __MSDOS__ */
/*    cp = rf->data; */
/*    blen = 0; */
/*    while ((got = fread(cp, 1, DOS_BIG_BUF, rf->f)) > 0) { */
/*	blen += got; */
/*	cp += got; */
/*    } */
/*    if (blen < rf->data_length) { */	/* only '<' because xmodem */
/*	rsr_err("Failed to read data section into large DOS buffer"); */
/*	return 0; */
/*    } */
/* #else */
    if (fread(rf->data, rf->data_length, 1, rf->f) != 1) {
	sprintf(rsr_errbuf, "Unable to read data_length bytes (%ld) from RSR file",
		rf->data_length);
	rsr_err(rsr_errbuf);
	ErrCode = ERR_SHORTFILE;
	return 0;
    }
/* #endif */
    rf->cdp = rf->data;

    /*
    ** very good. the only thing now is to decide how long each record
    ** is, and how many data are in each record, both daytime and night.
    */
    if (!rsr_rec_lengths(rf))
	return 0;

    /*
    ** now allocate space for the data record
    */
    if (!rsr_alloc_rec(rf))
	return 0;

    return 1;
}







/*
** rsr_rec_lengths
** determine how long the daytime and nighttime records are, and how
** many data items that represents. This should really only be called
** once, because the pointer to the 'current record' is allocated here.
** If called again, the original will live in the heap until the program
** terminates.
** This will set:
**	rf->rec_n_day		number of items during day
**	rf->rec_len_day		buffer byte length of daytime record
**	rf->rec_n_nite		number of items at night
**	rf->rec_len_nite	buffer byte length of night time record
**	rf->record		an RSR_rec pointer with its fields set up
** And if the file is earlier than Type2
**	rf->polar_day		character string composed of 0s and 1s
**	rf->polar_nite		which tell if the corresponding val is bipolar
*/
int rsr_rec_lengths(RSR_file *rf)
{
    int		find_poles(RSR_file *);

    if (rf->inst_type == Type2) {
	int	i, t, c, d, n;

	/* c = counter channels
	** d = daytime only channels
	** n = night time channels */
	c = d = n = 0;
	for (i=0; i<32; i++) {
	    if (rf->head->daytime[i]) d++;
	    if (rf->head->all_the_time[i]) n++;
	}
	for (i=0; i<6; i++)
	    if (rf->head->counter[i]) c++;

	/*
	** if the band is on,
	** add in 3 * the number of diodes to the daytime channels
	*/
	if (rf->head->band_on)
	    d += 3*rf->head->diodes;

	/*
	** number of nibbles from sign bits: (t+3)/4
	** number of nibbles from data: t*3
	** number of bytes: (sign_nibbles + data_nibbles + 1)/2
	*/
	rf->rec_n_day = t = c + n + d;
	rf->rec_len_day = ((t+3)/4 + t*3 + 1)/2;

	rf->rec_n_nite = t = c + n;
	if (t == 0)
	    rf->rec_len_nite = 0;
	else
	    rf->rec_len_nite = ((t+3)/4 + t*3 + 1)/2;

	rf->polar_day = NULL;
	rf->polar_nite = NULL;
    } else {
	int	i;

	if (rf->head->met_on)
	    rf->rec_n_nite =  rf->inst_type == Multi_Filter_32 ? 10 : 3;
	else
	    rf->rec_n_nite = 0;

	for (i=0; i<16; i++)
	    if (rf->head->extra[i])
		rf->rec_n_nite++;

	rf->rec_n_day = rf->rec_n_nite;

	if (rf->head->band_on)
	    if (rf->inst_type == Single_Channel)
		rf->rec_n_day += 4;
	    else
		rf->rec_n_day += 3*rf->head->diodes;

	rf->rec_len_day = ceil(rf->rec_n_day * 1.5);
	rf->rec_len_nite = ceil(rf->rec_n_nite * 1.5);

	if ((rf->polar_day = malloc(rf->rec_n_day)) == NULL) {
	    rsr_err("Memory low: no space for polar_day in rsr_rec_lengths");
	    return 0;
	}
	if (rf->rec_n_nite > 0) {
	    if ((rf->polar_nite = malloc(rf->rec_n_nite)) == NULL) {
		rsr_err("Mem low: no space for polar_nite in rsr_rec_lengths");
		free(rf->polar_day);
		return 0;
	    }
	} else
	    rf->polar_nite = NULL;
	    
	if (!find_poles(rf))
	    return 0;
    }

    if (rf->rec_len_nite == 0 && rf->rec_len_day == 0) {
	rsr_err("Day and night time both have 0 channels per record");
	return 0;
    }

    return 1;
}





/*
** allocate space for the single RSR_rec that represents
** the current record
*/
int rsr_alloc_rec(RSR_file *rf)
{

    if ((rf->record = malloc(sizeof(RSR_rec))) == NULL) {
	rsr_err("Memory low: no space for an RSR_rec in rsr_rec_lengths");
	return 0;
    }
    rf->record->obs_time = rf->start_time;
    rf->record->is_gap = 0;

    /*
    ** alloc space for the data part of the rf->record,
    ** size being the larger of the 2 determined in rsr_rec_lengths()
    */
    if ((rf->record->data = malloc(rf->rec_n_day*sizeof(RSR_datum))) == NULL) {
	sprintf(rsr_errbuf, "Memory allocation failed: "
			    "can't get %d daytime data items", rf->rec_n_day);
	rsr_err(rsr_errbuf);
	return 0;
    }

    /* "not read any yet" */
    rf->record->n_data = -2;

    return 1;
}







/*
** pre Type 2 files had a thing that allowed users to specify that
** channels were uni- or bi-polar. We need to know which is which for
** each item to be unpacked, so figure that out here.
*/
int find_poles(RSR_file *rf)
{
    int		i, d, n;

    if (rf->inst_type == Type2) {
	rsr_err("find_poles called with type 2 file");
	return 0;
    }

    /* the shadowband channels, if they're turned on... */
    if (rf->head->band_on)
	if (rf->inst_type == Single_Channel)
	    for (d=0; d<4; d++)
		rf->polar_day[d] = 0;
	else
	    for (d=0; d<3*rf->head->diodes; d++)
		rf->polar_day[d] = 0;
    else
	d = 0;

    n = 0;

    /* the met channels */
    if (rf->head->met_on)
	if (rf->inst_type != Multi_Filter_32)
	    for (i=0; i<3; i++) {
		rf->polar_day[d++] = 0;
		rf->polar_nite[n++] = 0;
	    }
	else {
	    for (i=0; i<10; i++) {
		rf->polar_day[d++] = 0;
		rf->polar_nite[n++] = 0;
	    }
	    rf->polar_day[d - 5] = 1;
	    rf->polar_day[d - 8] = 1;
	    rf->polar_nite[n - 5] = 1;
	    rf->polar_nite[n - 8] = 1;
	}

    /* the extra channels */
    for (i=0; i<16; i++)
	if (rf->head->extra[i]) {
	    rf->polar_day[d++] = rf->head->bipolar[i];
	    rf->polar_nite[n++] = rf->head->bipolar[i];
	}

    if (d != rf->rec_n_day) {
	sprintf(rsr_errbuf, "find_poles: should have %d daytime, got %d",
			rf->rec_n_day, d);
	rsr_err(rsr_errbuf);
	return 0;
    }

    if (n != rf->rec_n_nite) {
	sprintf(rsr_errbuf, "find_poles: should have %d nite, got %d",
			rf->rec_n_nite, n);
	rsr_err(rsr_errbuf);
	return 0;
    }

    return 1;
}






/*
** rsr_file_delete
*/
void rsr_file_delete(RSR_file *rf)
{
    if (rf == NULL)
	return;

    /* if calling program asked that *rf be left alone
       in the event of an error just return */

    if(!rf->deleteOnError) 
	return;

    rsr_rec_delete(rf->record);
    rf->record = NULL;

    if (rf->filename != NULL) {
	free(rf->filename);
	rf->filename = NULL;
    }

    if (rf->polar_day != NULL) {
	free(rf->polar_day);
	rf->polar_day = NULL;
    }

    if (rf->polar_nite != NULL) {
	free(rf->polar_nite);
	rf->polar_nite = NULL;
    }

    if (rf->head != NULL) {
	free(rf->head);
	rf->head = NULL;
    }

    if (rf->raw_head != NULL) {
	free(rf->raw_head);
	rf->raw_head = NULL;
    }

    if (rf->data != NULL) {
/*
#ifdef __MSDOS__
	farfree(rf->data);
#else
*/
	free(rf->data);
/* #endif __MSDOS__ */
	rf->data = NULL;
    }

    fclose(rf->f);
    free(rf);
}





/*
** rsr_next_record
** For the rsr file referred to by 'rf', unpack the next data
** record into an array of RSR_datum.
** The RSR_datum array is also part of rf; this function just
** returns that. The 'array' is a dynamically allocated block
** that gets malloced when the file header gets read and
** freed when the file header is deleted. The user should definately
** not be freeing this herself.
*/
RSR_rec *rsr_next_record(RSR_file *rf)
{
    int			reclen;
    void		rsr_unpack_record(RSR_file *, int);
    static long		skip = -1;
    unsigned long 	bytes_left;

    /*
    ** if the current data pointer is more than data_length bytes away
    ** from the start of the data buffer, then we ran past the end, which
    ** is fine, it just means the previous call got the last record (EOF).
    ** This can happen normally as an xmodem packet gets filled out to the 
    ** full 128 bytes after all the data in the buffer is exhausted.
    */
    if (rf->cdp - rf->data >= rf->data_length) {
        /* fprintf(stderr, "That's all folks.  cdp = %lu data = %lu data_length = %lu\n",
		(unsigned long) rf->cdp, (unsigned long) rf->data, rf->data_length); */
	rf->record->n_data = -1;
	return rf->record;
    }

    rf->record->is_gap = 0;

    reclen = *(rf->cdp);

    if (reclen == rf->rec_len_day || reclen == rf->rec_len_nite) {
        /* check to see if n bytes are available */

        bytes_left = rf->data_length - (unsigned long) rf->cdp + (unsigned long) rf->data;

        /* fprintf(stderr, "bytes left = %lu\n", bytes_left); */

        if(bytes_left < reclen) {
	   sprintf(rsr_errbuf, "Error in data section at record %lu: Truncated record."
			"\n\tExpecting %lu bytes, only %lu left in file.",
		    rf->curr_rec_no+1, reclen, bytes_left);
		rsr_err(rsr_errbuf);
	   rsr_err(rsr_errbuf);
	   rf->record->n_data = -1;
	   return rf->record;
        }

	/*
	 * at this point the record length indicator has checked out and there 
	 * are at least reclen bytes left in the file so we can unpack the current
	 * record, secure in the knowledge that there's something there to be unpacked.
	 */
		
	if (!rf->skip_unpack)
	    rsr_unpack_record(rf, reclen);

	rf->cdp += reclen + 1;
	rf->record->n_data = reclen;
    } 

    else if (reclen == 0xfe) {	/* gap record indicator */
	if (skip == -1) {
	    int		i;

	    skip = (rf->cdp[1] << 24) + (rf->cdp[2] << 16) +
					(rf->cdp[3] << 8) + rf->cdp[4];
	    if (skip <= 0) {
		sprintf(rsr_errbuf, "Error in data section at record %lu/byte %lu: "
		    "Gap record skip count: must be > 0",
		    rf->curr_rec_no+1, rf->cdp - rf->data + rf->header_length);
		rsr_err(rsr_errbuf);
		rf->record->n_data = -1;
		return NULL;
	    }
	    for (i=0; i<rf->rec_n_day; i++)
		rf->record->data[i] = -9999;
	}
	if (skip == 1) {
	    rf->cdp += 5;
	    skip = -1;
	} else
	    skip--;
	rf->record->is_gap = 1;
	rf->record->n_data = rf->rec_n_day;	/* should compute time... */
    } 

    else if (reclen == 0xff) {	/* split file end marker */
	rf->record->n_data = -1;
	return rf->record;
    } 

    else {
	sprintf(rsr_errbuf, "Error in data section at record %lu/byte %lu: "
	    "The current record length indicator (%d) doesn't make sense",
	    rf->curr_rec_no+1, rf->cdp - rf->data + rf->header_length, reclen);
	rsr_err(rsr_errbuf);
	rf->record->n_data = -1;
	return rf->record;
    }

    /*
    ** increment the current record number and compute
    ** the time of the current record.
    */
    rf->curr_rec_no++;
    rsr_set_current_time(rf);

    return rf->record;
}




void rsr_set_current_time(RSR_file *rf)
{
    rf->record->obs_time = rf->start_time +
				rf->head->avg_period*(rf->curr_rec_no - 1);
}





/*
** rsr_unpack_record
** fiddle the bits
*/
void rsr_unpack_record(RSR_file *rf, int n)
{
    void	rsr_unpack_1(RSR_file *, int),
		rsr_unpack_2(RSR_file *, int);

    if (n == 0)
	return;

    if (rf->skip_unpack) {
	int i;
 	for(i=0;i<n;i++) 
		rf->record->data[i] = -8888;
        return;
    }

    if (rf->inst_type == Type2)
	rsr_unpack_2(rf, n);
    else
	rsr_unpack_1(rf, n);

    return;
}







/*
** rsr_rec_delete
*/
void rsr_rec_delete(RSR_rec *r)
{
    if (r != NULL) {
	if (r->data != NULL) {
	    free(r->data);
	    r->data = NULL;
	}
	free(r);
    }
}





char	rsr_last_msgbuf[1024];

/*
** rsr_err
*/
void rsr_err(char *msg)
{
    strcpy(rsr_last_msgbuf, msg);
}



char *rsr_last_message(void)
{
    return rsr_last_msgbuf;
}


void rsr_clear_err(void)
{
    strcpy(rsr_last_msgbuf, "No errors");
}


















/*
** rsr_unpack_2
** unpack the next record from a type 2 file
*/
void rsr_unpack_2(RSR_file *rf, int n)
{
    int			i, nib, bm;
/*
#ifdef __MSDOS__
    unsigned char far	*p;
#else
*/
    unsigned char	*p;
/* #endif */

    /*
    ** as we enter, rf->cdp points to the record length
    **
    ** find the end of the sign bits.
    ** in type 2 files, sign bits are packed to the nearest nibble,
    ** followed by the packed 12-bit values. therefore, we gotta
    ** figure out which byte and nibble represents the start of the 12-bit
    ** data groups
    **
    ** p will point to the byte containing the first 12-bit group
    */
    p = rf->cdp + (n + 11)/8;

    /*
    ** nib will be 0 if the left (most sig.) nibble  of *p contains
    ** the most sig. nibble of the 12-bit group, and 1 if the right
    ** (least sig.) nibble contains it.
    */
    nib = (n - 1)%8 < 4;

    /*
    ** Now just decide where to start looking (left or right nib), then
    ** shift and mask as usual. Then increment p 1 or 2 bytes, and
    ** invert 'nib'.
    */
    for (i=0; i<n; i++) {
	if (nib) {		/* start w/least sig. nibble of this byte */
	    rf->record->data[i] = ((p[0] & 0x0f) << 8) | p[1];
	    p += 2;
	} else {		/* start w/most sig. nibble of this byte */
	    rf->record->data[i] = (p[1] >> 4) | (p[0] << 4);
	    p++;
	}
	nib = !nib;
    }

    /* magnitudes are done, now do the signs */
    p = rf->cdp + 1;
    for (bm=0x80, i=0; i<n; i++) {
	if (*p & bm)
	    rf->record->data[i] *= -1;
	if ((bm >>= 1) == 0) {
	    p++;
	    bm = 0x80;
	}
    }
}








/*
** rsr_unpack_1
** unpack the next record from a pre-type2 file
*/
void rsr_unpack_1(RSR_file *rf, int n)
{
    unsigned char	*p;
    char		*poles;
    int			i;

    p = rf->cdp + 1;

    for (i=0; i<n; i+=2, p+=3) {
	rf->record->data[i] = (p[1] >> 4) | (p[0] << 4);
	if (i+1 < n)
	    rf->record->data[i+1] = ((p[1] & 0x0f) << 8) | p[2];
    }

    if (n == rf->rec_n_day)
	poles = rf->polar_day;
    else
	poles = rf->polar_nite;

    for (i=0; i<n; i++)
	if (poles[i])
	    if (rf->record->data[i] >= 2048)
		rf->record->data[i] = 2 * (rf->record->data[i] - 4096);
	    else
		rf->record->data[i] *= 2;
}






/*
** Time functions
*/

#include <time.h>

#define HRS_PER_DAY     24L
#define SECONDS_PER_DAY    86400L
#define HRS_PER_YEAR    8760L
#define DAYS_1900_1970	25568L	/* days from 1/1/00 to 1/1/1970  */

#ifdef __MSDOS__
static char *rsr_def_format = "%H:%M:%S %a %b %d %Y";
#else
static char *rsr_def_format = "%H:%M:%S %a %h %d %Y";
#endif

/*
 * jday is an RSR style date (e.g., 34567.2345), buf is where you want the
 * output to go.  If fmt is NULL then one of the above formats is used.
 * Otherwise fmt is a user defined strftime() string.
 */

char *rsr_strftime(double jday, char *buf, int bufsize, char *fmt)
{
  time_t secs;
  char *format;
  struct tm *ts;

  if(fmt == (char *) NULL)
    format = rsr_def_format;
  else
    format = fmt;
    
  if((secs = rsr_j2unix(jday)) == -1)  /* something's wrong with jdays */
    return((char *) NULL);

  ts = gmtime(&secs);

  if(!strftime(buf, bufsize, format, ts)) {
    sprintf(rsr_errbuf, "rsr_strftime: strftime error.");
    rsr_err(rsr_errbuf);
    return((char *) NULL);
  }

  return(buf);
}
    

time_t rsr_j2unix(double jdays)
{
  time_t secs;

  /* if jdays is before Fri Feb 19 1982 or after Sat Nov 22 2036 then 
   * we say it is "suspicious" */

  if(jdays < DAYS_1900_1970) { 
    sprintf(rsr_errbuf, "rsr_j2unix: jdays value %f predates 1/1/70.", jdays);
    rsr_err(rsr_errbuf);
    return (-1);
  }

  jdays -= (double) DAYS_1900_1970;

  /* we add 0.5 to accomplish correct rounding when going from double to time_t */

  secs = (time_t) ((jdays * 86400.0) + 0.5);

  if(secs < 0) {
    sprintf(rsr_errbuf, "rsr_j2unix: negative secs value (%ld)", (long) secs);
    rsr_err(rsr_errbuf);
    return (-1);
  }
    
  return(secs);
}

double rsr_unix2j(time_t secs)
{
  double jdays;

  if(secs < 0) {
    sprintf(rsr_errbuf, "rsr_unix2j: negative secs value (%ld)\n", (long) secs);
    rsr_err(rsr_errbuf);
    return (-1.0);
  }
    
  jdays = (double) DAYS_1900_1970;
  jdays += ((double) secs)/SECONDS_PER_DAY;

  return(jdays);
}

#define MASK_BYTE_0	0x000000ffL
#define MASK_BYTE_1	0x0000ff00L
#define MASK_BYTE_2	0x00ff0000L
#define MASK_BYTE_3	0xff000000L

/*
** rsr_write_header()
**
** take an RSR_file pointer and write a header out 
** to file pointed to by fp.  fp is assumed to be open for
** writing and is left that way.  A rewind is done just for robustness.
**
** The way this function was intended to be used is for the user to fill
** in the "head" field (of type RSR_header) of the RSR_file struct, and
** just pass that struct through to rsr_write_header and let it figure
** out the translations.  The RSR_header structure is human decipherable
** and thus the programmer can just assign values to header fields via
** normal assignment rather than bit shifts, etc.
** 
** Example:
**
** rf->head->latitude -= 0.002;
** rf->head->unit_id = 0xbabe;
** rsr_write_header(rf, rf->f);
**
** Some fields are restricted because changing them would cause certain
** underlying assumptions about the file structure to change in weird
** ways.  For example, if you were to try to change the software rev.
** from 12 to 13.  
**
** NOTE: When you change *anything* about an RSR file, you should
**
**	1) set the bytes field in the header to 0
**	2) put a file signature at the end of the output file
**	   indicating what program/version created it.
**
** -jas
*/

int rsr_write_header(RSR_file *rf, FILE *fp)
{
  unsigned char	hdr[64], *flags, *id, *bytes, *dbsec, *dbdate,
		*hid, *lng, *lat, *auxs, *bps, *rate, *ave,
		*photocal, *photoff, *dauxs, *cntrs;
  int		int_lat, int_long;

  /* 
  ** make a copy of the raw header 
  ** it had better be initialized with the current header because we 
  ** won't be filling in *everything*
  */

  memcpy(hdr, rf->raw_head, rf->header_length);

  /*
  ** software rev and unit id are in the same location for types I and II
  ** so we can make these assignments without checking inst_type 
  **
  ** Actually, software rev souldn't be changed by the user.
  */

  ((raw_type_1 *) hdr)->sr = (unsigned char) rf->head->soft_rev; 
  id = ((raw_type_1 *) hdr)->id;
  id[0] = (char) ((rf->head->unit_id & 0x0000ff00L) >> 8);
  id[1] = (char) (rf->head->unit_id & 0x000000ffL);

  /*
  ** the struct fields bytes, dbsec and dbdate are all in
  ** each of the two header type structs (raw_type_1 & raw_type_2)
  ** but are in different locations.  We need to get our local
  ** pointers in the appropriate locations.
  */
  switch(rf->inst_type) {	
	case Type2 : 	
          		/* set the bytes parameter to 0 to identify 
			   this as a non-authenic RSR file */

			bytes  = ((raw_type_2 *) hdr)->bytes;
  			bytes[0] = bytes[1] = bytes[2] = 0;

			/* fields that are peculiar to type 2 files */

			hid = ((raw_type_2 *) hdr)->hid;
  			hid[0] = (char) ((rf->head->head_id & 0x0000ff00L) >> 8);
  			hid[1] = (char) (rf->head->head_id & 0x000000ffL);

			((raw_type_2 *) hdr)->ndiodes = (char) rf->head->diodes;

			dauxs = ((raw_type_2 *) hdr)->dauxs;
			dauxs[0] = (char) ((rf->head->raw_daytime & 0xff000000L) >> 24);
			dauxs[1] = (char) ((rf->head->raw_daytime & 0x00ff0000L) >> 16);
			dauxs[2] = (char) ((rf->head->raw_daytime & 0x0000ff00L) >> 8);
			dauxs[3] = (char) (rf->head->raw_daytime & 0x000000ffL);

			auxs = ((raw_type_2 *) hdr)->auxs;
			auxs[0] = (char) ((rf->head->raw_all_the_time & 0xff000000L) >> 24);
			auxs[1] = (char) ((rf->head->raw_all_the_time & 0x00ff0000L) >> 16);
			auxs[2] = (char) ((rf->head->raw_all_the_time & 0x0000ff00L) >> 8);
			auxs[3] = (char) (rf->head->raw_all_the_time & 0x000000ffL);

			cntrs = ((raw_type_2 *) hdr)->cntrs;
			cntrs[0] = (char) ((rf->head->raw_counter & 0x00ff0000L) >> 16);
			cntrs[1] = (char) ((rf->head->raw_counter & 0x0000ff00L) >> 8);
			cntrs[2] = (char) (rf->head->raw_counter & 0x000000ffL);

			((raw_type_2 *) hdr)->errs = rf->head->err;

			/* common fields */
			lat = ((raw_type_2 *) hdr)->lat;
			lng = ((raw_type_2 *) hdr)->lng;
			flags = &((raw_type_2 *) hdr)->flags;
			rate = ((raw_type_2 *) hdr)->rate;
			ave = ((raw_type_2 *) hdr)->ave;
			dbdate = ((raw_type_2 *) hdr)->dbdate;
			dbsec  = ((raw_type_2 *) hdr)->dbsec;

			break;

    	case Single_Channel:
    	case Multi_Filter_16 :
    	case Multi_Filter_32 :
			bytes  = ((raw_type_1 *) hdr)->bytes;
  			bytes[0] = bytes[1] = 0;

			bps = ((raw_type_1 *) hdr)->bps;
			bps[0] = (char) (((long) rf->head->raw_bipolar & 0x0000ff00L) >> 8);
			bps[1] = (char) (rf->head->raw_bipolar & 0x000000ffL);

			auxs = ((raw_type_1 *) hdr)->auxs;
			auxs[0] = (char) (((long) rf->head->raw_extra & 0x0000ff00L) >> 8);
			auxs[1] = (char) (rf->head->raw_extra & 0x000000ffL);

			photocal = ((raw_type_1 *) hdr)->photocal;
			photocal[0] = (char) ((rf->head->rsr_gain & 0x0000ff00L) >> 8);
			photocal[1] = (char) (rf->head->rsr_gain & 0x000000ffL);

			photoff = ((raw_type_1 *) hdr)->photoff;
			if(rf->head->diodes > 1) {
			  photoff[0] = (char) rf->head->diodes;
			  photoff[1] = (char) 0;
			}
			else {
			  photoff[0] = (char) ((rf->head->rsr_offset & 0x0000ff00L) >> 8);
			  photoff[1] = (char) (rf->head->rsr_offset & 0x000000ffL);
			}

			/* common fields */
			lat = ((raw_type_1 *) hdr)->lat;
			lng = ((raw_type_1 *) hdr)->lng;
			flags = &((raw_type_1 *) hdr)->flags;
			rate = ((raw_type_1 *) hdr)->rate;
			ave = ((raw_type_1 *) hdr)->ave;
			dbsec  = ((raw_type_1 *) hdr)->dbsec;
			dbdate = ((raw_type_1 *) hdr)->dbdate;

			break;

    	case Unknown_Instrument :
	default			: 
			rsr_err("rsr_write_header: Bad instrument type encountered");
			return(0);
  }

  /*
   * Now deal with common field names collectively.
   */

  /* translate latitude and longitude */

  int_lat = (int) (rf->head->latitude * 65536.0 / 360.0);
  int_long = (int) (rf->head->longitude * 65536.0 / 360.0);
  lat[0] = (char) ((int_lat & 0x0000ff00L) >> 8);
  lat[1] = (char) (int_lat & 0x000000ffL);

  lng[0] = (char) ((int_long & 0x0000ff00L) >> 8);
  lng[1] = (char) (int_long & 0x000000ffL);

  *flags = rf->head->flags;

  rate[0] = (char) ((rf->head->sample_rate & 0x0000ff00L) >> 8);
  rate[1] = (char) (rf->head->sample_rate & 0x000000ffL);

  ave[0] = (char) ((rf->head->avg_period & 0x0000ff00L) >> 8);
  ave[1] = (char) (rf->head->avg_period & 0x000000ffL);

  dbsec[0] = (char) ((rf->head->secs_today & 0x00ff0000L) >> 16);
  dbsec[1] = (char) ((rf->head->secs_today & 0x0000ff00L) >> 8);
  dbsec[2] = (char) (rf->head->secs_today & 0x000000ffL);

  dbdate[0] = (char) ((rf->head->days_1900 & 0x0000ff00L) >> 8);
  dbdate[1] = (char) (rf->head->days_1900 & 0x000000ffL);

  /* now we've changed the header and we're ready to write it out */

  /* if(fseek(rf->f, 0L, SEEK_SET) == -1) {
	sprintf(rsr_errbuf, "rsr_write_header: Problem rewinding file.");
	rsr_err(rsr_errbuf);
	return(0);
  } */

  if(fwrite(hdr, rf->header_length, 1, fp) != 1) {
	sprintf(rsr_errbuf, "rsr_write_header: Problem writing to file.");
	rsr_err(rsr_errbuf);
	return(0);
  }

  return(1);
}


/*
** rsr_write_records
** do a binary write of the file pointed to by the struct *rf
** from record 'from' to record 'to'.  fp is the output file
** pointer.  If rf == NULL, assume this is to be a gap record.
** If "to" is -1, then the block extends to end-of-file.
**
*/
int rsr_write_records(RSR_file *rf, long from, long to, FILE *fp)
{
  unsigned char	gapstr[5], *b, *e;
  long 		i, skipnum;
  int		bufsize;
  RSR_file	*temp;

  if(from < 0 || (to != -1 && from > to)) {
	sprintf(rsr_errbuf, 
		"rsr_write_records: got bad 'from' and 'to' parameters: %ld, %ld",
		from ,to);
	rsr_err(rsr_errbuf);
	return(0);
  }

  /* Handle gap record */
  if(rf == NULL) {
	skipnum = to - from + 1;   /* length of gap in records */
		
	gapstr[0] = 0xfe;
  	gapstr[1] = (unsigned char) ((skipnum & 0xff000000UL) >> 24);
  	gapstr[2] = (unsigned char) ((skipnum & 0x00ff0000L) >> 16);
  	gapstr[3] = (unsigned char) ((skipnum & 0x0000ff00) >> 8);
  	gapstr[4] = (unsigned char) (skipnum & 0x000000ff);

	if(fwrite(gapstr, 5, 1, fp) != 1) {
		rsr_err("rsr_write_records: couldn't write gap record");
		return(0);
	}
	
	return(1);
  }

  /*
  ** This is a waste of resources but at least it's robust
  */

  /* re-open input file */
  if((temp = rsr_open_file(rf->filename)) == NULL) 
	return(0);
  
  /* this is in case counters (or something else that would effect record
     lengths) have changed */

  temp->rec_len_day = rf->rec_len_day;
  temp->rec_n_day = rf->rec_n_day;
  temp->rec_len_nite = rf->rec_len_nite;
  temp->rec_n_nite = rf->rec_n_nite;

  /* don't really care what the record contents are */
  temp->skip_unpack = 1;

  /* we should now be at the begining of the data section of the input */

  /* 
  ** jump to the from record 
  ** if from == 0, then this loop doesn't get executed and
  ** cdp should still be pointing to the first byte (record 
  ** length) of the RSR file's data section.  Otherwise, we
  ** skip records until we get to 'from'.
  */

  for(i=0;i<from;i++)
	if(rsr_next_record(temp) == NULL) {
  		rsr_file_delete(temp);
		return(0);
	}

  /* note file pointer */
  b = temp->cdp;

  /* get end record */

  if(to == -1) {
	do {
		if(rsr_next_record(temp) == NULL) {
  		  rsr_file_delete(temp);
		  return(0);
		}
	} while(temp->record->n_data != -1); 
  }

  else {
     for(i=from;i<=to;i++)
	if(rsr_next_record(temp) == NULL) {
  		rsr_file_delete(temp);
		return(0);
	}
  }

  /* calculate buffer size */
  e = temp->cdp;
  bufsize = (int) (e - b);

  if(bufsize <= 0) {
  	sprintf(rsr_errbuf, "rsr_write_records: calculated bufsize <= 0 (%d, b=%d,e=%d)", bufsize, *b,*e);
  	rsr_err(rsr_errbuf);
  	rsr_file_delete(temp);
	return(0);
  }

  /* write block */
  if(!fwrite(b, bufsize, 1, fp)) {
	sprintf(rsr_errbuf, "rsr_write_records: couldn't write block (addr=%x bufsize=%d)", (unsigned) b, bufsize);
	rsr_err(rsr_errbuf);
  	rsr_file_delete(temp);
	return(0);
  }

  /* clean up */
  rsr_file_delete(temp);
  return(1);
}


/* 
** rsr_write_signature()
** writes 0xff tag at end of RSR file and puts file signature text
** after it for future identification purposes.
**
** should change this so multiple sigs can exist in one file 
*/

int rsr_write_signature(char *sig, FILE *fp)
{
   if(putc(0xff, fp) != 0xff || fprintf(fp, "%s", sig) == EOF ) {
	rsr_err("rsr_write_signature: Couldn't write to file");
	return(0);
   }

   return(1);
}


/*
** they got it right in solaris 2
*/
#if defined(SUNOS)
int	int_sprintf(char *str, const char *fmt, ...)
{
    char	*s;
    va_list	ap;

    va_start(ap, fmt);
    s = vsprintf(str, fmt, ap);
    va_end(ap);

    return strlen(s);
}
#else
#define	int_sprintf	sprintf
#endif




void rsr_print_header(RSR_file *r, char *buf, int buflen)
{
    char		*cp, *end, tbuf[64];
    static const char	*itypes[5] = {"unknown instrument type",
				"single channel", "multi-filter 16",
				"multi-filter 32", "type 2"};

    cp = buf;
    end = buf + buflen - 1;

    if (end - cp < 40) { strcpy(cp, "buf too small"); return; }
    cp += int_sprintf(cp, "This is a %s file. ", itypes[r->inst_type]);

    if (end - cp < 32) return;
    cp += int_sprintf(cp, "Latitude %.3fN, longitude %.3fW.\n", r->head->latitude,
						r->head->longitude);

    if (end - cp < 45) return;
    cp += int_sprintf(cp, "Sample rate %u seconds, averaging period %u seconds.\n",
		r->head->sample_rate, r->head->avg_period);

    if (end - cp < 64) return;
    cp += int_sprintf(cp, "%u quantities measured during the day, %u at night.\n",
		r->rec_n_day, r->rec_n_nite);

    if (!r->is_authentic) {
	if (end - cp < 35) return;
	cp += int_sprintf(cp, "File appears to be post-processed. ");
    }

    if (end - cp < 55) return;
    rsr_strftime(rsr_unix2j(r->start_time), tbuf, 64, NULL);
    cp += int_sprintf(cp, "Started at %s.\n", tbuf);
}






char *my_strerror(int n)
{
#ifndef sparc
    return strerror(n);
#else
    extern char	*sys_errlist[];

    return sys_errlist[n];
#endif
}

