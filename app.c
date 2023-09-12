/*
** $Id: app.c,v 1.15 2011/01/24 00:07:19 rolf Exp $
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

#include "globals.h"
#include "rMPI_model.h"
#include "phases.h"
#include "app.h"


/*
** Return the elapsed time
*/
double
app_model(int verbose, double tau, double checkpoint_time, double restart_time,
	double work_time, double ras_delay, FILE *fp_ints, FILE *fp_faults,
	float soft_time_to_reboot, float soft_reboot_success_rate, int hotswap)
{

double last_event;
double next_interrupt;
double time_left;
double rework_time;
double rework_done;
double elapsed_time;

int done;
int dead_nodes;


    /*
    ** First start
    */
    elapsed_time= 0.0;
    rework_time= 0.0;
    rework_done= 0.0;
    last_event= 0.0;


    /* 
    ** Generate monotonically increasing times at which the application
    ** experiences a fault and has to restart.
    */
    next_interrupt= rMPI(verbose, fp_ints, fp_faults, elapsed_time, soft_time_to_reboot,
			soft_reboot_success_rate, hotswap);

    while (next_interrupt < (last_event + ras_delay))   {
	/*
	** Often a whole bunch of faults occur at almost the same time while the
	** application is dying. Wait here for a moment until the (some of) the
	** burst has passed.
	*/
	next_interrupt= rMPI(verbose, fp_ints, fp_faults, elapsed_time, soft_time_to_reboot,
			    soft_reboot_success_rate, hotswap);
    }
    elapsed_time= elapsed_time + ras_delay;
    total_ras_delay= total_ras_delay + ras_delay;

    /* We expect the input to be monotonically increasing (and be > 0) */
    assert(next_interrupt >= last_event);

    /* Remember the last interrupt */
    last_event= next_interrupt;
    interrupt_cnt++;

    if (verbose > 1)   {
	fprintf(stderr, "%12.1f\" ------- Next interrupt (number %d) at %12.1f\" (%12.2f hours)\n",
	    elapsed_time, interrupt_cnt, next_interrupt, next_interrupt / 60.0);
    }

    done= do_work(next_interrupt, work_time, &rework_time, tau, tau, checkpoint_time, verbose, &elapsed_time);



    /*
    ** Now start looping over restart, rework, and work
    ** Checkpoints are written during do_work
    */
    while (!done)   {

	/* When will the next interrupt occur? */
	next_interrupt= rMPI(verbose, fp_ints, fp_faults, elapsed_time, soft_time_to_reboot,
			    soft_reboot_success_rate, hotswap);
	while (next_interrupt < (last_event + ras_delay))   {
	    /*
	    ** Often a whole bunch of faults occur at almost the same time while the
	    ** application is dying. Wait here for a moment until the (some of) the
	    ** burst has passed.
	    */
	    next_interrupt= rMPI(verbose, fp_ints, fp_faults, elapsed_time, soft_time_to_reboot,
				soft_reboot_success_rate, hotswap);
	}
	elapsed_time= elapsed_time + ras_delay;
	total_ras_delay= total_ras_delay + ras_delay;

	/* Remember the last interrupt */
	last_event= next_interrupt;
	interrupt_cnt++;

	if (verbose > 1)   {
	    fprintf(stderr, "%12.1f\" ------- Next interrupt (number %d) at %12.1f\" (%12.2f hours)\n",
		elapsed_time, interrupt_cnt, next_interrupt, next_interrupt / 60.0);
	}


	do_restart(next_interrupt, restart_time, verbose, &elapsed_time);
	if (elapsed_time >= next_interrupt)   {
	    /* Enter next cycle */
	    continue;
	}

	rework_done= do_rework(next_interrupt, rework_time, verbose, &elapsed_time);
	time_left= tau - rework_done;
	assert(time_left >= 0.0);

	if (elapsed_time >= next_interrupt)   {
	    /* Enter next cycle */
	    continue;
	}

	done= do_work(next_interrupt, work_time, &rework_done, time_left, tau, checkpoint_time, verbose, &elapsed_time);
	rework_time= rework_done;
    }

    /* Correct for overshooting */
    if ((work_time - total_work_time)  < 0.0)   {
	fprintf(stderr, "We have to correct elapsed time by %.3g\"\n", work_time - total_work_time);
	elapsed_time= elapsed_time - (work_time - total_work_time);
	total_work_time= work_time;
    } else if ((work_time - total_work_time)  > 0.0)   {
	fprintf(stderr, "We did not work enough by %.3g\"\n", work_time - total_work_time);
    }
    if (verbose > 1)   {
	fprintf(stderr, "%12.1f\" Work DONE:               %12.1f\" (%12.1fh)\n", elapsed_time,
	    total_work_time, total_work_time / 60.0);
    }

    /* Count how many faults we had in the last phase.  */
    dead_nodes= count_dead_nodes(elapsed_time, fp_faults);

    /*
    ** The last interrupt is really the end of the job, but we want to record
    ** the number of faults in the last phase.
    */
    if (fp_ints)   {
	fprintf(fp_ints, "%15.3f %d\n", last_event, dead_nodes);
    }

    return elapsed_time;

}  /* end of app_model() */
