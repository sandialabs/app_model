/*
** $Id: timing.c,v 1.4 2011/01/03 18:47:05 rolf Exp $
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

#include "globals.h"
#include "timing.h"



double
get_clock_value(void)
{

#if _POSIX_C_SOURCE >= 199309L
    #include <time.h>
    struct timespec tm;
    int rc;

    rc= clock_gettime(CLOCK_REALTIME, &tm);
    if (rc)   {
	perror("get_clock_value() failed");
    }
    return (double)(tm.tv_sec) + (double)(tm.tv_nsec) / 1000000000.0;

#else

    #include <sys/time.h>
    struct timeval tm;
    gettimeofday(&tm, NULL);
    return tm.tv_sec + tm.tv_usec * 1e-6;

#endif

}  /* end of get_clock_value() */



char *
disp_time(double value)
{

static char str[64];
int hours;
int minutes;
double seconds;


    hours= value / 3600.0;
    minutes= (value - (hours * 3600.0)) / 60.0;
    seconds= value - (hours * 3600.0) - (minutes * 60.0);

    sprintf(str, "%2dh:%02dm:%02.6fs", hours, minutes, seconds);
    return str;

}  /* end of disp_time() */
