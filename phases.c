/*
** $Id: phases.c,v 1.10 2011/01/03 21:51:23 rolf Exp $
**
** Rolf Riesen, September 2009 -- 2010, Sandia National Laboratories
** All time values are stored in doubles and are in minutes
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
#include <assert.h>
#include "phases.h"
#include "globals.h"

#define MIN(a, b)		((a) < (b) ? (a) : (b))



/*
** Do a restart
** Parameters:
**     IN	next_interrupt	Time when the next interrupt will occur
**     IN	restart_time	How much time it takes to do a restart
**     IN	verbose		Controls amount of debug output
**     IN/OUT	elapsed_time	Current time
*/
void
do_restart(double next_interrupt, double restart_time, int verbose, double *elapsed_time)
{
    if (next_interrupt > (*elapsed_time + restart_time))   {
	/*
	** We have enough time to finish this restart before the
	** next interrupt occurs.
	*/
	*elapsed_time= *elapsed_time + restart_time;
	restart_cnt++;
	total_restart_time= total_restart_time + restart_time;

	if (verbose > 3)   {
	    fprintf(stderr, "%12.1f\" restart time             %12.1f\", count %d\n", *elapsed_time,
		restart_time, restart_cnt);
	}

    } else   {
	/*
	** We will get interrupted again while restarting
	** Advance current time and try again.
	*/
	wasted_restart_time= wasted_restart_time + (next_interrupt - *elapsed_time);
	*elapsed_time= next_interrupt;
	failed_restart_cnt++;

	if (verbose > 2)   {
	    fprintf(stderr, "%12.1f\" restart %d/%d failed\n", *elapsed_time, failed_restart_cnt, restart_cnt);
	}
    }

}  /* end of do_restart() */



/*
** Do rework
** Parameters:
**     IN	next_interrupt	Time when the next interrupt will occur
**     IN	rework_time	How much rework is there to do?
**     IN	tau		Time between checkpoints
**     IN	verbose		Controls amount of debug output
**     IN/OUT	elapsed_time	Current time
**
** Return how much work we have done (could be good or wasted work)
*/
double
do_rework(double next_interrupt, double rework_time, int verbose, double *elapsed_time)
{

double rework_done;


    /*
    ** How much rework can we do, until the next interrupt
    ** or the next checkpoint?
    */
    rework_done= MIN((next_interrupt - *elapsed_time), rework_time);
    *elapsed_time= *elapsed_time + rework_done;

    /* Has the interrupt arrived? */
    if (*elapsed_time >= next_interrupt)   {
	/* Rework was interrupted */
	wasted_rework_time= wasted_rework_time + rework_done;
	failed_rework_cnt++;
	if (verbose > 2)   {
	    fprintf(stderr, "%12.1f\" rework time (partial)    %12.1f/%.0f\", count %d\n", *elapsed_time,
		rework_done, MIN((next_interrupt - *elapsed_time), rework_time), failed_rework_cnt);
	}
    } else if (rework_done >= rework_time)   {
	/* We are done with rework */
	total_rework_time= total_rework_time + rework_done;
	/*
	** If we make it through the rest of the segment to the next checkpoint,
	** rework_done will count as work done, but not yet!
	*/
	rework_cnt++;
	if (verbose > 3)   {
	    fprintf(stderr, "%12.1f\" rework (saved) done      %12.1f\", work done so far %12.1f\", count %d\n",
		*elapsed_time, rework_done, total_work_time, rework_cnt);
	}
    } else   {
	/* This means we had more rework than tau! */
	assert(FALSE);
    }

    return rework_done;

}  /* end of do_rework() */



/* 
** Resume work until next interrupt (and do checkpoints)
** Parameters:
**     IN	next_interrupt	Time when the next interrupt will occur
**     IN	work_time	Total amount of work that needs to be done
**     IN/OUT	rework_time	How much time has been done in this segment, and
**                              how much time will need to be done in next segment
**     IN	time_left_	How much time until checkpoint?
**     IN	checkpoint_time	How much time to write a checkpoint
**     IN	verbose		Controls amount of debug output
**     IN/OUT	elapsed_time	Current time
**
** Return false, if there is more work to do.
*/
int
do_work(double next_interrupt, double work_time, double *rework_time,
	double time_left_this_segment, double tau, double checkpoint_time,
	int verbose, double *elapsed_time)
{

double work_done;
double work_left;
double checkpoint_done;
int first_segment;


    first_segment= TRUE;
    while (TRUE)   {
	/*
	** How much work can we do, until the next interrupt
	** or the next checkpoint?
	*/
	work_left= work_time - total_work_time;
	if (first_segment)   {
	    /* Also add in the rework time done earlier in this segment */
	    work_left= work_left - *rework_time;
	}

	work_done= MIN(time_left_this_segment, (next_interrupt - *elapsed_time));
	work_done= MIN(work_done, work_left);
	*elapsed_time= *elapsed_time + work_done;

	if (first_segment)   {
	    /* Also add in the rework time done earlier in this segment */
	    work_done += *rework_time;
	    first_segment= FALSE;
	}

	/* Has the interrupt arrived? */
	if (*elapsed_time >= next_interrupt)   {
	    failed_work_cnt++;
	    *rework_time= work_done;
	    if (verbose > 2)   {
		fprintf(stderr, "%12.1f\" work time (partial)      %12.1f/%.0f\", count %d\n", *elapsed_time,
		    work_done, MIN(time_left_this_segment, work_left), failed_work_cnt);
	    }
	    break;
	}

	/* Must be checkpoint time */
	if (next_interrupt > (*elapsed_time + checkpoint_time))   {
	    /* We have time to write a checkpoint */
	    work_cnt++;

	    total_work_time= total_work_time + work_done;

	    if ((work_time - total_work_time) <= 0.0)   {
		/* We are done with work */
		if (verbose > 3)   {
		    fprintf(stderr, "%12.1f\" work (saved) DONE        %12.1f\", so far %12.1f\", count %d\n", *elapsed_time,
			work_done, total_work_time, work_cnt);
		    }
		return TRUE;  /* done */
	    }

	    *elapsed_time= *elapsed_time + checkpoint_time;
	    checkpoint_cnt++;
	    total_checkpoint_time= total_checkpoint_time + checkpoint_time;
	    *rework_time= 0.0;
	    time_left_this_segment= tau;
	    if (verbose > 3)   {
		fprintf(stderr, "%12.1f\" work (saved) time        %12.1f\", so far %12.1f\", count %d\n", *elapsed_time,
		    work_done, total_work_time, work_cnt);
		fprintf(stderr, "%12.1f\" checkpoint time          %12.1f\", count %d\n", *elapsed_time,
		    checkpoint_time, checkpoint_cnt);
	    }
	} else   {
	    /* We will get interrupted during a checkpoint write */
	    checkpoint_done= next_interrupt - *elapsed_time;
	    *elapsed_time= *elapsed_time + checkpoint_done;
	    wasted_checkpoint_time= wasted_checkpoint_time + checkpoint_done;
	    failed_checkpoint_cnt++;

	    *rework_time= work_done;
	    if (verbose > 2)   {
		fprintf(stderr, "%12.1f\" work (not saved) time    %12.1f\", count %d\n", *elapsed_time,
		    work_done, failed_work_cnt);
		fprintf(stderr, "%12.1f\" failed checkpoint time   %12.1f/%.0f\", count %d\n", *elapsed_time,
		    checkpoint_done, checkpoint_time, failed_checkpoint_cnt);
	    }
	    break;
	}
    }

    if ((work_time - total_work_time) > 0.0)   {
	return FALSE; /* not done */
    } else   {
	return TRUE;  /* done */
    }

}  /* end of do_work() */
