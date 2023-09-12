/*
** $Id: input.c,v 1.4 2010/12/23 00:34:09 rolf Exp $
**
** Rolf Riesen, September 2009 -- 2010, Sandia National Laboratories
** Read fault times from an input file
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
#include "input.h"

#define MAX_ERR_STR_LEN	(2 * 1024)


static int max_nodes= 0;
static FILE *fp_in= NULL;



int
init_input(FILE *fp_input, int num_bundles)
{
    if (fp_input == NULL)   {
	/* We are not reading from a file */
	return FALSE;
    }

    /* Store the file point and max number of nodes */
    max_nodes= num_bundles;
    fp_in= fp_input;

    return TRUE;

}  /* end of init_input() */



/*
** Extract the next time value from the input file.
** The file is supposed to contain three fields: time node error
** Time is in seconds, node is a rank number >= 0, and error
** is a single word error indication.
*/
double
read_next(int verbose)
{

int rc;
double t;
static double start;
int node;
char err[MAX_ERR_STR_LEN];
static int first_time= TRUE;


    if (first_time)   {
	/*
	** The first time around we read the first line and use that time as an offset.
	** That way the fault data and the application both start at 0.
	*/
	first_time= FALSE;
	rc= fscanf(fp_in, "%lf %d %s", &start, &node, err);
	if (rc == EOF)   {
	    return -1;
	} else if (rc != 3)   {
	    fprintf(stderr, "ERROR: Input file unparsable\n");
	    return -1;
	}
    }


    /*
    ** Make sure the node is within our node count.
    ** Basically, the simulator runs an application on num_bundles nodes.
    ** The input file represents a machine that may have more than that
    ** many nodes. The simulator "runs" the application on the first num_bundles
    ** nodes.
    */
    while (TRUE)   {
	rc= fscanf(fp_in, "%lf %d %s", &t, &node, err);
	if (rc == EOF)   {
	    return -1;
	} else if (rc != 3)   {
	    fprintf(stderr, "ERROR: Input file unparsable\n");
	    return -1;
	}

	read_input_cnt++;

	if (node < max_nodes)   {
	    /* Acceptable node */
	    break;
	}
    }

    /* Convert to minutes */
    t= (t - start) / 60.0;
    if (verbose > 2)   {
	fprintf(stderr, "Input file %12.3f\"\n", t);
    }

    read_input_accepted++;
    return t;

}  /* end of read_next() */
