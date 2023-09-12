/*
** $Id: rMPI_model.h,v 1.14 2011/01/24 00:07:19 rolf Exp $
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
#ifndef _RMPI_MODEL_H
#define _RMPI_MODEL_H


void rMPI_init(int num_bundles, int total_nodes, FILE *fp_input, int verbose);

double
rMPI(int verbose, FILE *fp, FILE *fp_faults, double elapsed_time,
	float soft_time_to_reboot, float soft_reboot_success_rate,
	int hotswap);

int count_dead_nodes(double elapsed_time, FILE *fp_faults);

#endif /* _RMPI_MODEL_H */
