#ifndef CALIB_H

#define CALIB_H

#include <time.h>

#include "rsrlib.h"

typedef struct {
    char	*name, *units;
    int		index, order;
    double	*coefs;
} cal_el;


typedef struct c_t {
    time_t	date;
    int		n_cals, source_line_no;
    char	*id_string;
    cal_el	*cals;
    struct c_t	*next;
} cal_t;


void	delete_calib(cal_t *),
	calibrate(RSR_file *, cal_t *, double *);

#endif
