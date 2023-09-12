/*
** $Id: input.h,v 1.4 2010/12/23 00:34:09 rolf Exp $
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
#ifndef _INPUT_H
#define _INPUT_H


int init_input(FILE *fp_input, int num_bundles);
double read_next(int verbose);

#endif /* _INPUT_H */
