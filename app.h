/*
** $Id: app.h,v 1.9 2011/01/24 00:07:19 rolf Exp $
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
#ifndef _APP_H_
#define _APP_H_

double
app_model(int verbose, double tau, double checkpoint_time, double restart_time,
	double work_time, double ras_delay, FILE *fp, FILE *fp_faults,
	float soft_time_to_reboot, float soft_reboot_success_rate,
	int hotswap);

#endif /* _APP_H_ */
