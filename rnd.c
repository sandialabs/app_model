/*
** $Id: rnd.c,v 1.9 2011/01/24 00:07:19 rolf Exp $
**
** Rolf Riesen, September 2009 -- 2010, Sandia National Laboratories
**
** This file is part of app_model. App_model is free software and
** is distributed under the terms of the GNU General Public License
** Version 3. See the file LICENSE for details.
** Copyright 2009 Sandia Corporation. Under the terms of Contract
** DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
** retains certain rights in this software.
**
*/
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>

#include "globals.h"
#include "rnd.h"

static rnd_t _rnd= RND_EXP;
static gsl_rng *_r;
static double _node_mtbf= 43800; /* Five years */
static double _dist_shape; /* Shape distribution parameter */
static double _dist_scale; /* Scale parameter for Weibull */



void
init_rnd(rnd_t rnd, double node_mtbf, int default_seed, double shape, double scale)
{

const gsl_rng_type *T;


    _rnd= rnd;
    _node_mtbf= node_mtbf;
    _dist_shape= shape;
    _dist_scale= node_mtbf / gsl_sf_gamma(1.0 + 1.0 / shape);
    _dist_scale= scale;

    gsl_rng_env_setup();
    T= gsl_rng_default;
    _r= gsl_rng_alloc(T);
    if (default_seed == FALSE)   {
	gsl_rng_set(_r, time(NULL) + getpid());
    }

}  /* end of init_rnd() */



double
next_node_failure(double start_time)
{

    rnd_gen_cnt++;
    switch (_rnd)   {
	case RND_EXP:
	    return start_time + gsl_ran_exponential(_r, _node_mtbf);

	case RND_GAMMA:
	    return start_time + gsl_ran_gamma(_r, _dist_shape, _node_mtbf);

	case RND_WEIBULL:
	    return start_time + gsl_ran_weibull(_r, _dist_scale, _dist_shape);

	default:
	    fprintf(stderr, "Unknown random number distribution requested!\n");
	    exit(6);
    }

}  /* end of next_node_failure() */



double
rnd_probability(void)
{
    rnd_prob_cnt++;
    return gsl_ran_flat(_r, 0.0, 1.0);
}  /* end of rnd_probability() */
