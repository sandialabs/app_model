/*
** $Id: phases.h,v 1.6 2010/12/23 23:57:58 rolf Exp $
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
#ifndef _PHASES_H_
#define _PHASES_H_

void
do_restart(double next_interrupt, double restart_time, int verbose,
	double *elapsed_time);

double
do_rework(double next_interrupt, double rework_time, int verbose,
	double *elapsed_time);

int
do_work(double next_interrupt, double work_time, double *rework_time, double time_left_this_segment,
	double tau, double checkpoint_time, int verbose, double *elapsed_time);

#endif /* _PHASES_H_ */
