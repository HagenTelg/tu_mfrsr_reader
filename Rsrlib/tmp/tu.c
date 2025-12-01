#include <stdio.h>
#include <stdlib.h>

#include "tu.h"

/*
** 'The Unpacker'
**
** This program reads one or more RSR dump files and emits an
** ASCII version of the same.  This version is based
** on a portable unpacking library developed by
** Handmade Software.
**
** November 11, 1994, Handmade Software Inc.
** version 1.00
**
** 12/14/94 rs
** Add -c/coscor processing parameter that indicates a file to use
** for cosine-correcting the data. If none given, no CC is done. If
** a filename is provided which is in any way incorrect, processing stops.
** version 1.1
**
** 5/3/95 rs
** Remove the '-q' flag and replace its functionality with a new
** option to the 'verbosity' flag/config: 'quiet'. The command line
** now accepts '-v quiet' which will act just like '-q' used to.
** 'quiet' is also accepted in the config file.
**
** 5/12/95 rs
** When the 'header output' is selected, tu would not distinguish
** between single-channel and multifilter files. Lee changed the
** meaning of 2 bytes in the header slightly, and so I changed
** emit_header() in unpack.c to output the header slightly differently
** depending on the input file.
**
** 6/26/95 rs
** 'setup_pp' has always bugged me; working on a replacement; if accepted,
** version will be 1.2.2
**
** 8/1/95 js 
** Code in rsr_rec_length() in rsrlibc.c was incorrectly calculating
** the recordlength anytime the number of channels was a multiple
** of 8.  i.e., when nc=8, record length should be 13, old code
** calculated 14. Version 1.2.3
**
** 8/2/95 rs
** Negative values in type 2 files computed incorrectly in library.
**
** 10/27/95 rs
** The revised 'setup_pp' incorrectly processed "extra flags". Fixed.
** A couple of things (uninit corr_kind and 0 or 1 key) in Solarinfo fixed.
** Version 1.2.5
**
** 1/24/96 rs
** Embarking on a major change to incorporate calibration.
** The plan is to add a command line flag and corresponding
** config file entry which indicates that the calibration is to be done,
** and the file to find the calib. data in.
** See the file "calib.txt" for a more
** complete technical description of the method. When complete, a
** major version change is expected, probably 1.3...
**
** 3/20/96 (solstice!) rs
** Getting reasonable numbers from the calibration code. Will need
** tuning and bullet-protection, but so far it's promising. Making
** a version change to 1.2.901.
**
** 4/29/96 rs
** Got a good report from a user of calibration. Also a bug report
** concerning the "date=rsr" format. See 'unpack.c' for the details.
** version 1.2.902.
**
** 6/24/96 rsr
** Re-fixed (?) a bug in mystrtod.c. 1.2.903
**
** 9/19/96 rs
** A bit of a miscalculation in 'do_unpack' about when "fmt4" and "sstoa"
** should be called. Fixed. 1.2.904
**
** 10/24/96 rs
** Changed the order that calibration and cosine correction are called.
** Fixed problem in calibration which referred to raw counts instead of
** corrected (direct, cosine, etc) data. 1.2.905
**
** 10/24/96 rs
** Compiling under DJGPP gives core dump when run as "tu -h"; some
** member variables in pp are unitialized, but this is fixed by setting
** to NULL in ini_pp.
** Also pp->fnms was uninitialized which can cause a core dump when
** freeing if there were no files to unpack. v 1.2.906
**
** 10/29/96 rs
** There was a test in cosine_correct which made sure that direct was
** high enough to do something with; unfortuantely, after calibrating,
** that number may be very small. changed the limit from 1 to 0.0001.
** v 1.2.907
**
** 11/7/96 rs
** Change to interpretation of "error byte" in librsr.
** v 1.2.908
**
** 6/29/98 js
** No change to code, just a reordering of Makefiles and version number stuff.
** Now, version numbers for tar files are keyed of the #define VERSION.  I'm
** trying to make this standard for all ASRC Solar group software (that comes
** from me, anyway).
** v 1.2.909
**
** 7/20/98 js
** in cos.c commented out code that swithced global and beam data.  Rob must have
** done this to make things orthogonal and then forgot he had done it becuase as
** it turns out, the cosine correction code assumes the b d g ordering for single
** channel units.  Thus, the swapping was screwing things up royally.
** v 1.2.910
**
*/

#define VERSION "1.2.910"

char *Version = VERSION;

/*
** the overall processing parameters are described in the
** global variable 'pp'
proc_params	pp;
*/

Verbosity	do_msg_output = Unset;
extern long	timezone;
#ifndef sparc
extern int	daylight;
#endif

int main(int argc, char **argv)
{
    proc_params	*pp,
		*setup_pp(int, char **);
    int		rv,
		do_unpack(proc_params *);
    void	usage(char *, proc_params *);

    timezone = 0L;
#ifndef sparc
    daylight = 0;
#endif

    /* read default, cfg file, and command line parameters */
    if ((pp = setup_pp(argc, argv)) == NULL)
	exit(1);

    /* help? */
    if (pp->help.u_val.ival) {
	usage(argv[0], pp);
	pp_delete(pp);
	return 0;
    }

    /* version? */
    if (pp->version.u_val.ival) {
	char	buf[128];

	sprintf(buf, "%s version %s", argv[0], pp->version.value);
	tu_msg(buf);

	pp_delete(pp);
	return 0;
    }

    rv = do_unpack(pp);

    pp_delete(pp);

    return rv;
}



void tu_advise(char *msg)
{
    if (do_msg_output == Verbose)
	tu_msg(msg);
}


void tu_msg(char *msg)
{
    if (do_msg_output != Quiet && do_msg_output != QuietCmdLine) {
	fputs(msg, stderr);
	fputc('\n', stderr);
    }
}




void pp_delete(proc_params *pp)
{
    param	*p[16];
    int		i;

    if (pp == NULL)
	return;

    p[0] = &pp->timezone;
    p[1] = &pp->cfg_fnm;
    p[2] = &pp->verbosity;
    p[3] = &pp->help;
    p[4] = &pp->version;
    p[5] = &pp->nighttime;
    p[6] = &pp->separator;
    p[7] = &pp->date_fmt;
    p[8] = &pp->time_fmt;
    p[9] = &pp->report_time;
    p[10] = &pp->out_fnm;
    p[11] = &pp->gap;
    p[12] = &pp->extra;
    p[13] = &pp->coscor_fnm;
    p[14] = &pp->header;
    p[15] = &pp->calibration;

    for (i=0; i<16; i++)
	if (p[i] != NULL) {
	    if (p[i]->parameter != NULL) free(p[i]->parameter);
	    if (p[i]->description != NULL) free(p[i]->description);
	    if (p[i]->def_value != NULL) free(p[i]->def_value);
	    if (p[i]->cfg_name != NULL) free(p[i]->cfg_name);
	    if (p[i]->value != NULL) free(p[i]->value);
	    /*
	    ** do this for strings and extra only!
	    if (p[i]->u_val != NULL) free(p[i]->u_val);
	    */
	}

    if (pp->fnms != NULL) {
	for (i=0; i<pp->n_files_to_unpack; i++)
	    if (pp->fnms[i] != NULL)
		free(pp->fnms[i]);
	free(pp->fnms);
    }

    free(pp);
}




/*
** usage()
** print usage message
*/
void usage(char *exe, proc_params *pp)
{
    char	buf[512];
    param	*params[17], **p;

    sprintf(buf, "Usage: %s [flags] fnm [fnm...]", exe);
    tu_msg(buf);
    tu_msg("Unpack the RSR files 'fnm'...\nFlags:");

    params[0] = &pp->timezone;
    params[1] = &pp->cfg_fnm;
    params[2] = &pp->verbosity;
    params[3] = &pp->help;
    params[4] = &pp->version;
    params[5] = &pp->nighttime;
    params[6] = &pp->separator;
    params[7] = &pp->date_fmt;
    params[8] = &pp->time_fmt;
    params[9] = &pp->report_time;
    params[10] = &pp->out_fnm;
    params[11] = &pp->gap;
    params[12] = &pp->extra;
    params[13] = &pp->coscor_fnm;
    params[14] = &pp->header;
    params[15] = &pp->calibration;
    params[16] = NULL;

    for (p=params; *p != NULL; p++) {
	sprintf(buf, "-%c: %s", (*p)->cmdln_char, (*p)->description);
	tu_msg(buf);
    }
}





