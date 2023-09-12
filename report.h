/*
** $Id: report.h,v 1.10 2010/12/21 23:32:32 rolf Exp $
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
#ifndef _REPORT_H_
#define _REPORT_H_

void
report_results(double work_time, double elapsed_time, double calculated_sys_mtbf,
	double calculated_app_mtbf, int display_perf_info, double model_time,
	double daly, FILE *fp_input, double calculated_fpi);

#endif /* _REPORT_H_ */
