#ifndef TU_H

#define TU_H

typedef enum {
    Quiet=0,
    Normal,
    Verbose,
    Unset,
    QuietCmdLine
} Verbosity;
typedef enum {Doy0=0, Doy1, Mdy, Dmy, Unix, Rsr, Joe, Ymd} Date_fmt;
typedef enum {Hms=0, Hours, Daysec} Time_fmt;
typedef enum {Start=0, Middle, End} Report_time;
typedef enum {AzD=0, AzR, ElD, ElR, ZenD, ZenR, HaD, HaR,
		DecD, DecR, Am, Soldst, Tst, Done} Extra;

typedef union {
    double	dval;
    char	*sval;
    int		ival;
    Verbosity	verb_val;
    Date_fmt	date_val;
    Time_fmt	time_val;
    Report_time	rep_val;
    Extra	*extra_val;
} u_type;

typedef struct tu_param {
    char	*parameter,
		*description,
		*def_value,
		*cfg_name,
		cmdln_char,
		*value;
    int		(*validate)(struct tu_param *, int);
    u_type	u_val;
} param;


typedef struct {
    int		n_files_to_unpack;
    char	**fnms;
    param	timezone,
		cfg_fnm,
		verbosity,
		help,
		version,
		header,
		nighttime,
		separator,
		date_fmt,
		time_fmt,
		report_time,
		out_fnm,
		gap,
		extra,
		coscor_fnm,
		calibration;
} proc_params;


void	pp_delete(proc_params *),
	tu_advise(char *),
	tu_msg(char *);


#endif	/* TU_H */
