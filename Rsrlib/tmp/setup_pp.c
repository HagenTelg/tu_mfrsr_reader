#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>

#include "tu.h"

#if defined(__GO32__) || defined(__MSDOS__)
#define	DEFAULT_CFG_FNM		"tu.ini"
#else
#define DEFAULT_CFG_FNM		".turc"
#endif

/* a case-insensitive string compare, like stricmp */
int	ig_strcmp(char *, char *);

/*
** setup_pp
**
** determine the processing parameters for the program.
** There are 3 stages of parameter determination:
** 1. First, there is a default value for every parameter. These
** are set here.
** 2. Next, a configuration file (if it exists) is opened and read
** which overides the default values.
** 3. Finally, the user can provide values on the command line
** which take precedence over everything.
**
** 6/22/95 rs
** The previous version of this file/function was clumsy and hard to
** maintain. Here is a new version that is clumsy and less hard to
** maintain.
**
** 8/3/95 rs
** setup_pp was returning satisfactorily when no files to work on
** were mentioned on the command line. this has changed so that no
** files is an error condition. Jim told me to do it.
**
** 10/27/95 rs
** copy_extra never did what it supposed to (after the rewrite).
** fixed it.
**
** 1/24/96 rs
** Add support for a "do calibration" parameter.
**
** 10/24/96 rs
** Unitialized member of pp->verbosity causing core dump on DOS
** under DJGCC. Set value to NULL in ini_pp.
** Unitialized value of pp->fnms causes core dump when free if no fnms.
** v1.2.906
**
** 10/29/96 rs
** Fixed coscos limit check in unpack:cosine_correct(); see comment in tu.c
** 1.2.907
**
*/


typedef struct pp_sym {
    char		*ini_name,
			cmdline_flag,
			*def_val, *cfg_val, *cmdline_val;
    int			has_arg, ok_cmdline, ok_config;
    struct pp_sym	*next;
} pp_symbol;

extern char *Version;

#define		PP_SYMTAB_SIZE	17

#define		OK_CMDLINE	0x100
#define		OK_CONFIG	0x200
#define		NO_ARG		0x400

pp_symbol	*pp_symtab[PP_SYMTAB_SIZE],
		*locate_sym(char *, char *, int);


proc_params *setup_pp(int argc, char **argv)
{
    int		initialize_pp_symtab(void),
		pre_scan(int, char **, proc_params *),
		real_scan(proc_params *);
    void	close_symtab(void),
		print_pp(proc_params *),
		setup_pp_names(proc_params *);
    proc_params	*pp;

    if (!initialize_pp_symtab())
	return NULL;

    if ((pp = malloc(sizeof(proc_params))) == NULL) {
	tu_msg("Memory allocation failure in setup_pp");
	close_symtab();
	return NULL;
    }

    pp->n_files_to_unpack = 0;
    pp->fnms = NULL;

    if (!pre_scan(argc, argv, pp)) {
	close_symtab();
	return NULL;
    }

    setup_pp_names(pp);

    if (!real_scan(pp)) {
	free(pp);
	pp = NULL;
    } else
	print_pp(pp);

    close_symtab();

    return pp;
}





int initialize_pp_symtab(void)
{
    if (locate_sym("timezone", "0", OK_CMDLINE + OK_CONFIG + 'z') == NULL)
	return 0;
    if (locate_sym("configfile", DEFAULT_CFG_FNM, OK_CMDLINE + 'f') == NULL)
	return 0;
    if (locate_sym("verbosity", "normal", OK_CMDLINE + OK_CONFIG + 'v') == NULL)
	return 0;
    if (locate_sym("help", "no", NO_ARG + OK_CMDLINE + 'h') == NULL)
	return 0;
    if (locate_sym("version", Version, NO_ARG + OK_CMDLINE + 'V') == NULL)
	return 0;
    if (locate_sym("nighttime", "-9999", OK_CMDLINE + OK_CONFIG + 'n') == NULL)
	return 0;
    if (locate_sym("separator", " ", OK_CMDLINE + OK_CONFIG + 's') == NULL)
	return 0;
    if (locate_sym("date", "doy1", OK_CMDLINE + OK_CONFIG + 'd') == NULL)
	return 0;
    if (locate_sym("time", "hours", OK_CMDLINE + OK_CONFIG + 't') == NULL)
	return 0;
    if (locate_sym("reptime", "end", OK_CMDLINE + OK_CONFIG + 'r') == NULL)
	return 0;
    if (locate_sym("outfile", "stdout", OK_CMDLINE + OK_CONFIG + 'o') == NULL)
	return 0;
    if (locate_sym("gap", "-9998", OK_CMDLINE + OK_CONFIG + 'g') == NULL)
	return 0;
    if (locate_sym("extra", NULL, OK_CMDLINE + OK_CONFIG + 'x') == NULL)
	return 0;
    if (locate_sym("coscor", NULL, OK_CMDLINE + OK_CONFIG + 'c') == NULL)
	return 0;
    if (locate_sym("header", "no", NO_ARG+OK_CMDLINE+OK_CONFIG + 'H') == NULL)
	return 0;
    if (locate_sym("calibration", NULL, OK_CMDLINE + OK_CONFIG + 'l') == NULL)
	return 0;

    return 1;
}




int pre_scan(int ac, char **av, proc_params *pp)
{
    int		pre_scan_cmdline(int, char **, proc_params *),
		pre_scan_config(void);

    if (!pre_scan_cmdline(ac, av, pp))
	return 0;

    if (!pre_scan_config())
	return 0;

    return 1;
}



int pre_scan_cmdline(int ac, char **av, proc_params *pp)
{
    int		i, not_unpacking;
    char	buf[128];
    extern int	do_msg_output;
    pp_symbol	*pps,
		*lookup_pp_sym_by_flag(int);

    /*
    ** highest priority is quietness; if user asked for '-v quiet', then
    ** we gotta honor it.
    */
    for (i=1; i<ac; i++)
	if (av[i][0] == '-' && av[i][1] == 'v')
	    if (i+1 < ac && ig_strcmp(av[i+1], "quiet") == 0)
		do_msg_output = QuietCmdLine;

    /* will have to test if this is still 0 after checking all the flags */
    pp->n_files_to_unpack = 0;

    /* this will get set if version or help is asked for */
    not_unpacking = 0;

    /*
    ** now check for other strange items
    */
    for (i=1; i<ac; i++)
	if (av[i][0] == '-') {
	    if ((pps = lookup_pp_sym_by_flag(av[i][1])) == NULL) {
		sprintf(buf, "tu: setup_pp: unknown flag '%c'; stopping.",
				av[i][1]);
		tu_msg(buf);
		return 0;
	    }

	    if (!pps->ok_cmdline) {
		sprintf(buf, "The %c flag is not allowed on the command line",
				av[i][1]);
		tu_msg(buf);
		return 0;
	    }

	    if (pps->has_arg) {
                if (i >= ac - 1) {
                    sprintf(buf, "The %c flag must be followed by an option",
                                av[i][1]);
                    tu_msg(buf);
                    return 0;
                } else {
		    pps->cmdline_val = strdup(av[i+1]);
		    i++;	/* skip the arg */
		}
	    } else
		pps->cmdline_val = strdup("yes");

	    /* so it's a hack */
	    if (pps->cmdline_flag == 'V' || pps->cmdline_flag == 'h')
		not_unpacking++;
	} else {
	    int		j;

	    pp->n_files_to_unpack = ac - i;
	    if ((pp->fnms = malloc((ac-i) * sizeof(char *))) == NULL) {
		sprintf(buf, "Memory alloc fail; %d filenames\n", ac-i);
		tu_msg(buf);
		return 0;
	    }
	    for (j=0; i<ac; j++, i++)
		pp->fnms[j] = strdup(av[i]);
	    break;
	}

    /*
    ** normally, if no files are given, it's an error. but if they asked for
    ** version or help, then it's OK
    */
    if (!not_unpacking && pp->n_files_to_unpack == 0) {
	sprintf(buf, "No files to unpack\n");
	tu_msg(buf);
	return 0;
    }

    return 1;
}





/*
** load up the config files entries into the symbol table
** -- queue error/warnings in an array until the file is all read;
** we don't want to spew messages if there's a 'quiet' verbosity
** directive in the config file.
*/
int pre_scan_config(void)
{
    FILE	*f, *find_cfg_file(int *);
    char	*new_sym, new_val[128],
		err_buf[16][128],
		*get_next_sym(FILE *, int *, char *);
    int		line, cfg_open, err_no = 0;
    pp_symbol	*pp;
    extern int	do_msg_output;

    if ((f = find_cfg_file(&cfg_open)) == NULL)
	return cfg_open;

    line = 0;
    while ((new_sym = get_next_sym(f, &line, new_val)) != NULL) {
	if ((pp = locate_sym(new_sym, NULL, 0)) == NULL) {
	    if (err_no < 16)
		sprintf(err_buf[err_no],
			"config file: unknown symbol '%s' ignored",
			new_sym);
	    err_no++;
	    continue;
	}

	if (new_val[0] != '\0')		/* if it's "", leave it NULL */
	    pp->cfg_val = strdup(new_val);
	else
	    pp->cfg_val = NULL;
    }

    fclose(f);

    /*
    ** now check to see if we can emit any err messages; examine 'verbosity'
    */
    if (do_msg_output != QuietCmdLine) {
	if ((pp = locate_sym("verbosity", NULL, 0)) == NULL) {
	    tu_msg("pre_scan_config: 'verbosity' disappeared: fatal!");
	    return 0;
	}
	if (ig_strcmp(pp->cfg_val, "quiet") != 0) {
	    int	i;

	    for (i=0; i<err_no; i++)
		tu_msg(err_buf[i]);
	    if (err_no > 16)
		tu_msg("...more errors detected but not reported.");
	} else
	    do_msg_output = Quiet;
    }

    return 1;
}





/*
** locate and open the config file; this will look for the symbol
** table entry for "configfile" and check if there is an alternate
** one entered on command line; if so, and it can't be opened, complain
** and return failure. if not, look for the default name - if it's
** not around, don't complain, just indicate that things are ok by making
** retcode be 1; if things fail and something is really wrong, the return
** value is NULL and retcode is 0.
*/
FILE *find_cfg_file(int *retcode)
{
    pp_symbol 	*pp;
    FILE	*f;
    char	*s;

    *retcode = 0;

    if ((pp = locate_sym("configfile", NULL, 0)) == NULL) {
	tu_msg("setup_pp: 'configfile' symbol disappeared");
	return NULL;
    }

    /*
    ** if they give an alt config file and it ain't there, then complain;
    ** don't bother if looking for the default named one.
    */
    if (pp->cmdline_val != NULL)
	s = pp->cmdline_val;
    else {
	s = pp->def_val;
	*retcode = 1;
    }

    if ((f = fopen(s, "r")) == NULL) {
    	if (*retcode == 0) {	/* i.e., it's a command-line fnm */
	    char	buf[128];

	    sprintf(buf, "setup_pp: can't open config file '%s'", s);
	    tu_msg(buf);
	}

	return NULL;
    }

    return f;
}




/*
** the supposed config file 'f' is open and waiting...read it until a line
** like "sym = val" is found; return the symbol, copy the value to val.
*/
char *get_next_sym(FILE *f, int *line_no, char *val)
{
    static char	buf[512];
    char	*p,
		*cleanup_line(char *);
    int		empty_string(char *);
    void	cleanup_string(char *);

    while (fgets(buf, 512, f) != NULL) {
	(*line_no)++;

	cleanup_line(buf);
	if (empty_string(buf))
	    continue;

	if ((p = strchr(buf, '=')) == NULL) {
	    sprintf(buf, "config file, line %d; missing '='; line ignored",
					*line_no);
	    tu_msg(buf);
	    continue;
	}

	strcpy(val, p+1);
	*p = '\0';
	p = buf;

	cleanup_string(p);
	cleanup_string(val);

	return p;
    }

    return NULL;
}



/*
** clean up the string s by removing leading and trailing white space
** do this by MOVING the CHARS, not by moving the pointer...
*/
void cleanup_string(char *s)
{
    char	*p;

    /* clean the end */
    p = strrchr(s, '\0');
    for (p--; p > s; p--)
	if (*p == ' ' || *p == '\t')
	    *p = '\0';
	else
	    break;

    if (p == s)	/* this shouldn't happen */
	return;

    /* now find the beginning... */
    for (p=s; *p == ' ' || *p == '\t'; p++) {
    }

    /* if the first and last characters are `"' (and different!) then rm */
    if (*p == '"') {
	char	*last;

	last = strrchr(p, '\0') - 1;
	if (*last == '"' && p != last) {
	    *last = '\0';
	    p++;
	}
    }

    /*
    memmove(s, p, strlen(p) + 1);
    */
    while (*s++ = *p++) {
    }

}







/*
** remove comments and/or newline from s
*/
char *cleanup_line(char *s)
{
    char	*p;

    if ((p = strchr(s, '#')) != NULL)
	*p = '\0';

    if ((p = strchr(s, '\n')) != NULL)
	*p = '\0';

    return s;
}





/*
** return true iff p contains only blanks and tabs
*/
int empty_string(char *p)
{
    while (*p) {
	if (*p != ' ' && *p != '\t')
	    return 0;
	p++;
    }

    return 1;
}





/*
** put all the 'parameter' and 'desc' fields in the pp
*/
void setup_pp_names(proc_params *pp)
{
    void	ini_pp(param *, char *, char *, char *);

    ini_pp(&pp->help, "help",
		"Help", "Print help message then exit");
    ini_pp(&pp->version, "version",
		"Version", "Print version number");
    ini_pp(&pp->timezone, "timezone",
		"Time zone", "Hours west of Greenwich");
    ini_pp(&pp->cfg_fnm, "configfile",
		"Config file", "Configuration file");
    ini_pp(&pp->verbosity, "verbosity", 
		"Verbosity", "Quantity of informational messages");
    ini_pp(&pp->header, "header",
		"Header", "Insert RSR header in output file");
    ini_pp(&pp->nighttime, "nighttime",
		"Nighttime", "String to print for RSR quantities at night");
    ini_pp(&pp->separator, "separator", 
		"Separator", "String printed between data columns");
    ini_pp(&pp->date_fmt, "date",
		"Date format", "Style to print record date");
    ini_pp(&pp->time_fmt, "time", 
		"Time format", "Style to print record time");
    ini_pp(&pp->report_time, "reptime",
		"Report time", "Reported time of record");
    ini_pp(&pp->out_fnm, "outfile",
		"Output filename", "File to write converted data to");
    ini_pp(&pp->gap, "gap",
		"Gap processing", "Gap field string");
    ini_pp(&pp->extra, "extra",
		"Extra info", "Calculated solar quantities");
    ini_pp(&pp->coscor_fnm, "coscor",
	"Correction filename", "Name of cosine correction (solarinfo) file");
    ini_pp(&pp->calibration, "calibration",
		"Calibration filename", "File to read calibration data from");
}




void ini_pp(param *p, char *name, char *par, char *desc)
{
    pp_symbol	*pps;

    if ((pps = locate_sym(name, NULL, 0)) == NULL) {
	char	buf[128];
	sprintf(buf, "fatal: ini_pp failed to locate %s", name);
	tu_msg(buf);
	return;
    }

    p->parameter = strdup(par);
    p->description = strdup(desc);
    p->cfg_name = strdup(pps->ini_name);
    p->cmdln_char = pps->cmdline_flag;
    p->value = NULL;
    p->def_value = NULL;
}





/*
** copy default and user input from the symbol table to pp
*/
int real_scan(proc_params *pp)
{
    int		rv,
		copy_help(param *), copy_version(param *),
		copy_timezone(param *), copy_cfg_fnm(param *),
		copy_verbosity(param *), copy_header(param *),
		copy_nighttime(param *), copy_separator(param *),
		copy_date_fmt(param *), copy_time_fmt(param *),
		copy_report_time(param *), copy_out_fnm(param *),
		copy_gap(param *), copy_extra(param *),
		copy_coscor_fnm(param *),
		copy_calibration(param *);

    rv = copy_help(&pp->help);
    if (rv == -1)	/* error occurred */
	return 0;
    else if (rv == 1)	/* just help, please */
	return 1;

    rv = copy_version(&pp->version);
    if (rv == -1)	/* error occurred */
	return 0;
    else if (rv == 1)	/* just version, please */
	return 1;

    if (!copy_timezone(&pp->timezone)) return 0;
    if (!copy_cfg_fnm(&pp->cfg_fnm)) return 0;
    if (!copy_verbosity(&pp->verbosity)) return 0;
    if (!copy_header(&pp->header)) return 0;
    if (!copy_nighttime(&pp->nighttime)) return 0;
    if (!copy_separator(&pp->separator)) return 0;
    if (!copy_date_fmt(&pp->date_fmt)) return 0;
    if (!copy_time_fmt(&pp->time_fmt)) return 0;
    if (!copy_report_time(&pp->report_time)) return 0;
    if (!copy_out_fnm(&pp->out_fnm)) return 0;
    if (!copy_gap(&pp->gap)) return 0;
    if (!copy_extra(&pp->extra)) return 0;
    if (!copy_coscor_fnm(&pp->coscor_fnm)) return 0;
    if (!copy_calibration(&pp->calibration)) return 0;

    return 1;
}





/*
** copy the relevant stuff from a symbol table entry to a proc_param entry
*/
void copy_sym_to_param(pp_symbol *pps, param *p)
{
    /* def value not used */
    p->def_value = NULL;

    if (pps->cmdline_val != NULL)
	p->value = strdup(pps->cmdline_val);
    else if (pps->cfg_val != NULL)
	p->value = strdup(pps->cfg_val);
    else if (pps->def_val != NULL)
	p->value = strdup(pps->def_val);
    else
	p->value = NULL;
}




int list_compare(char *s, const char *list[])
{
    int         i;

    for (i=0; list[i] != NULL; i++)
	if (ig_strcmp(s, (char *) list[i]) == 0)
	    break;

    return i;
}





/*
** return -1 if there's a problem; return 0 if 'no help'; return 1 if 'help'
*/
int copy_help(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("help", NULL, 0)) == NULL)
	return -1;

    copy_sym_to_param(pps, p);
    
    /* supposed to be assignment here */
    return p->u_val.ival = !ig_strcmp(p->value, "yes");
}




/*
** return:
** -1 if problem
** 0 if 'no version'
** 1 if 'just version, please'
*/
int copy_version(param *pp)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("version", NULL, 0)) == NULL)
	return -1;

    /* copy the symbol table 'default value' to the pp->value; */
    pp->value = strdup(pps->def_val);
    
    /* supposed to be assignment here */
    return pp->u_val.ival = !ig_strcmp(pps->cmdline_val, "yes");
}



static char	errbuf[512];

int copy_timezone(param *p)
{
    pp_symbol	*pps;
    double	tz, f, i;

    if ((pps = locate_sym("timezone", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (sscanf(p->value, "%lf", &tz) != 1) {
	sprintf(errbuf, "Time zone value (%s) isn't a number", p->value);
	tu_msg(errbuf);
	return 0;
    }

    if (tz < -23.5 || tz > 23.5) {
	sprintf(errbuf, "Time zone value (%.2f) must be -23.5..23.5", tz);
	tu_msg(errbuf);
	return 0;
    }

    /* make sure the fractional part is 0 or 0.5 */
    f = modf(tz, &i);
    f = fabs(f);
    p->u_val.dval = tz;
    if (f > 0.499 && f < 0.501)
	return 1;
    else if (f < 0.001)
	return 1;
    else {
	sprintf(errbuf,
		    "fractional part of time zone (%f) must be .0 or .5", f);
	tu_msg(errbuf);
	return 0;
    }
}



/*
** this has already been taken care of, but put it in the pp anyway
*/
int copy_cfg_fnm(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("configfile", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    p->u_val.sval = strdup(p->value);

    return 1;
}



int copy_verbosity(param *p)
{
    int			i;
    pp_symbol		*pps;
    extern int		do_msg_output;
    static const char	*verbs[] = {"quiet", "normal", "verbose", NULL};

    if ((pps = locate_sym("verbosity", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    p->u_val.sval = strdup(p->value);
    i = list_compare(p->value, verbs);
    do_msg_output = i;
    if (verbs[i] == NULL) {
	sprintf(errbuf, "time value (%s) is not valid", p->value);
	tu_msg(errbuf);
	return 0;
    } else {
	p->u_val.verb_val = i;
	return 1;
    }
}


int copy_header(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("header", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    /*
    p->u_val.sval = strdup(p->value);
    */
    p->u_val.ival = ig_strcmp(p->value, "no");

    return 1;
}



int copy_nighttime(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("nighttime", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (ig_strcmp(p->value, "omit") == 0)
	p->u_val.sval = NULL;
    else
	p->u_val.sval = strdup(p->value);

    return 1;
}



int copy_separator(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("separator", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (ig_strcmp(p->value, "format") == 0)
	p->u_val.sval = NULL;
    else
	p->u_val.sval = strdup(p->value);

    return 1;
}



int copy_date_fmt(param *p)
{
    pp_symbol		*pps;
    int			i;
    static const char   *pos[] = {"doy0", "doy1", "mdy", "dmy",
				  "unix", "rsr", "joe", "ymd", NULL};

    if ((pps = locate_sym("date", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    i = list_compare(p->value, pos);
    p->u_val.date_val = i;
    if (pos[i] == NULL) {
	sprintf(errbuf, "date value (%s) is not valid", p->value);
	tu_msg(errbuf);
	return 0;
    } else
	return 1;
}




int copy_time_fmt(param *p)
{
    pp_symbol		*pps;
    int			i;
    static const char   *pos[] = {"hms", "hours", "daysec", NULL};

    if ((pps = locate_sym("time", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    i = list_compare(p->value, pos);
    p->u_val.time_val = i;
    if (pos[i] == NULL) {
	sprintf(errbuf, "time value (%s) is not valid", p->value);
	tu_msg(errbuf);
	return 0;
    } else
	return 1;
}



int copy_report_time(param *p)
{
    pp_symbol           *pps;
    int			i;
    static const char   *pos[] = {"start", "middle", "end", NULL};

    if ((pps = locate_sym("reptime", NULL, 0)) == NULL)
        return 0;

    copy_sym_to_param(pps, p);

    i = list_compare(p->value, pos);
    p->u_val.rep_val = i;
    if (pos[i] == NULL) {
        sprintf(errbuf, "report-time value (%s) is not valid", p->value);
        tu_msg(errbuf);
        return 0;
    } else
        return 1;
}



int copy_out_fnm(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("outfile", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (ig_strcmp(p->value, "stdout") == 0)
	p->u_val.sval = NULL;
    else
	p->u_val.sval = strdup(p->value);

    return 1;
}



int copy_gap(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("gap", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    p->u_val.sval = strdup(p->value);

    return 1;
}



int copy_extra(param *p)
{
    pp_symbol	*pps;
    int		i;
    char	*s;

    if ((pps = locate_sym("extra", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (p->value == NULL) {
	p->u_val.extra_val = NULL;
    } else {
	p->u_val.extra_val = malloc((strlen(p->value)+1)*sizeof(Extra));
	if (p->u_val.extra_val == NULL) {
	    sprintf(errbuf, "Error: ran out of mem copying extras");
	    tu_msg(errbuf);
	    return 0;
	}
	for (i=0, s=p->value; *s; s++) {
	    switch (*s) {
	    case 'a':
		p->u_val.extra_val[i++] = AzD; break;
	    case 'A':
		p->u_val.extra_val[i++] = AzR; break;
	    case 'e':
		p->u_val.extra_val[i++] = ElD; break;
	    case 'E':
		p->u_val.extra_val[i++] = ElR; break;
	    case 'z':
		p->u_val.extra_val[i++] = ZenD; break;
	    case 'Z':
		p->u_val.extra_val[i++] = ZenR; break;
	    case 'h':
		p->u_val.extra_val[i++] = HaD; break;
	    case 'H':
		p->u_val.extra_val[i++] = HaR; break;
	    case 'd':
		p->u_val.extra_val[i++] = DecD; break;
	    case 'D':
		p->u_val.extra_val[i++] = DecR; break;
	    case 'm':
	    case 'M':
		p->u_val.extra_val[i++] = Am; break;
	    case 's':
	    case 'S':
		p->u_val.extra_val[i++] = Soldst; break;
	    case 't':
	    case 'T':
		p->u_val.extra_val[i++] = Tst; break;
	    default:
		sprintf(errbuf,
		    "Warning: unknown extra value (%c) ignored.", *s);
		tu_msg(errbuf);
		break;
	    }
	}
	p->u_val.extra_val[i] = Done;
    }

    return 1;
}



int copy_coscor_fnm(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("coscor", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    p->u_val.sval = p->value != NULL ? strdup(p->value) : NULL;

    return 1;
}




int copy_calibration(param *p)
{
    pp_symbol	*pps;

    if ((pps = locate_sym("calibration", NULL, 0)) == NULL)
	return 0;

    copy_sym_to_param(pps, p);

    if (p->value == NULL) {
	p->u_val.sval = NULL;
    } else {
	p->u_val.sval = strdup(p->value);
	if (p->u_val.sval == NULL) {
	    sprintf(errbuf, "Error: ran out of mem copying calibration filename");
	    tu_msg(errbuf);
	    return 0;
	}
    }

    return 1;
}







/*
** print_pp()
** spew all the processing parameters
*/
void print_pp(proc_params *pp)
{
    param	*p[17], **lp;
    char	buf[512];
    int		i;

    if (ig_strcmp(pp->verbosity.value, "quiet") == 0 ||
		ig_strcmp(pp->verbosity.value, "normal") == 0)
	return;

    sprintf(buf, "%d file%s to unpack (",
	pp->n_files_to_unpack, pp->n_files_to_unpack > 1 ? "s" : "");
    /* buf[0] = '\0'; why did i put this here? */
    for (i=0; i<pp->n_files_to_unpack; i++) {
	if (strlen(buf) + strlen(pp->fnms[i]) > 500) {
	    strcat(buf, "...)");
	    break;
	}

	strcat(buf, pp->fnms[i]);
	if (i == pp->n_files_to_unpack - 1)
	    strcat(buf, ")");
	else
	    strcat(buf, ", ");
    }
    tu_advise(buf);

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
    p[16] = NULL;

    for (lp=p; *lp != NULL; lp++)
	if ((*lp)->description != NULL) {
	    sprintf(buf, "%-32.32s: '", (*lp)->description);
	    if ((*lp)->value != NULL)
		strcat(buf, (*lp)->value);
	    else
		strcat(buf, "(empty)");
	    strcat(buf, "'");

	    tu_advise(buf);
	}
}






pp_symbol *lookup_pp_sym_by_flag(int flag)
{
    int		i;
    pp_symbol	*p;

    for (i=0; i<PP_SYMTAB_SIZE; i++)
	for (p=pp_symtab[i]; p != NULL; p = p->next)
	    if (p->cmdline_flag == flag)
		return p;

    return NULL;
}





/*
** return pointer to the symbol 's'; if 'add_flag' is true, then add the symbol
** if it's not already in the table; if add_flag is false, then return NULL if
** not in the table.
**
** if the symbol DOES get added to the table, then set its 'def_val'
** structure member to the parameter 'val'.
**
** note that add_flag does double duty here - it indicates whether to add
** the string to the table, and also is the command-line flag associated
** with the symbol. i.e., it gets added, too.
*/
pp_symbol *locate_sym(char *s, char *val, int add_flag)
{
    int		hv,
		pp_hash(char *);
    pp_symbol	*p;

    hv = pp_hash(s);
    for (p = pp_symtab[hv]; p != NULL; p = p->next)
	if (ig_strcmp(s, p->ini_name) == 0)
	    return p;

    /* not in table */
    if (add_flag) {
	if ((p = malloc(sizeof(pp_symbol))) == NULL) {
	    tu_msg("setup_pp: memory allocation failure, proc_params");
	    return NULL;
	}
	p->ini_name = strdup(s);
	/* sunos dumps core when strdup-ing NULL */
	p->def_val = val ? strdup(val) : NULL;
	p->cfg_val = NULL;
	p->cmdline_val = NULL;
	p->cmdline_flag = add_flag & 0xff;
	p->has_arg = !(add_flag & NO_ARG);
	p->ok_cmdline = add_flag & OK_CMDLINE;
	p->ok_config = add_flag & OK_CONFIG;
	p->next = pp_symtab[hv];
	pp_symtab[hv] = p;
	return p;
    } else
	return NULL;
}




int pp_hash(char *s)
{
    int		c;
    char	*p;

    for (c=0, p=s; *p; p++)
	c += *p;

    return c % PP_SYMTAB_SIZE;
}





void close_symtab(void)
{
    int		i;
    pp_symbol	*pps, *t;

    for (i=0; i<PP_SYMTAB_SIZE; i++)
	for (pps=pp_symtab[i]; pps != NULL; ) {
	    free(pps->ini_name);
	    if (pps->def_val)
		free(pps->def_val);
	    if (pps->cfg_val != NULL)
		free(pps->cfg_val);
	    if (pps->cmdline_val != NULL)
		free(pps->cmdline_val);

	    t = pps->next;
	    free(pps);
	    pps = t;
	}
}


int ig_strcmp(char *s1, char *s2)
{
    if (s1 == NULL)
	if (s2 == NULL)
	    return 0;
	else
	    return 1;
    else
	if (s2 == NULL)
	    return 1;

    while (*s1 != '\0') {
	if (toupper(*s1) != toupper(*s2))
	    return 1;
	s1++;
	s2++;
    }
    return *s2 != '\0';
}






#ifdef TEST

int do_msg_output;

int main(int ac, char **av)
{
    (void) setup_pp(ac, av);

    return 0;
}

void tu_msg(char *s)
{
    fputs(s, stderr);
    fputs("\n", stderr);
}

void tu_advise(char *s)
{
    fputs("Advise: ", stderr);
    fputs(s, stderr);
    fputs("\n", stderr);
}

#endif
