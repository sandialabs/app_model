/*
** $Id: rnd.h,v 1.5 2011/01/24 00:07:19 rolf Exp $
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
#ifndef _RND_H_
#define _RND_H_

/* Which random number distribution to use */
typedef enum {RND_EXP, RND_GAMMA, RND_WEIBULL} rnd_t;


void init_rnd(rnd_t rnd, double node_mtbf, int default_seed, double shape, double scale);
double next_node_failure(double start_time);
double rnd_probability(void);


#endif /* _RND_H_ */
