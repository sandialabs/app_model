/*
** $Id: globals.c,v 1.5 2010/12/21 23:32:32 rolf Exp $
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



/* Some global time keepers */
double total_restart_time;
double total_rework_time;
double total_work_time;
double total_checkpoint_time;
double total_ras_delay;

double wasted_restart_time;
double wasted_rework_time;
double wasted_work_time;
double wasted_checkpoint_time;

/* Global counters */
int checkpoint_cnt, failed_checkpoint_cnt;
int restart_cnt, failed_restart_cnt;
int rework_cnt, failed_rework_cnt;
int work_cnt, failed_work_cnt;
int interrupt_cnt;
int fault_cnt;
int node_failure_cnt;
int total_repaired;
int soft_reboot_success_cnt;
int soft_reboot_failure_cnt;

int rnd_gen_cnt;
int rnd_prob_cnt;
int calls_rMPI;
int read_input_cnt;
int read_input_accepted;



void
init_globals(void)
{
    /* Times */
    total_restart_time= 0.0;
    total_rework_time= 0.0;
    total_work_time= 0.0;
    total_checkpoint_time= 0.0;
    total_ras_delay= 0.0;

    wasted_restart_time= 0.0;
    wasted_rework_time= 0.0;
    wasted_work_time= 0.0;
    wasted_checkpoint_time= 0.0;


    /* Counters */
    checkpoint_cnt= 0;
    failed_checkpoint_cnt= 0;
    restart_cnt= 0;
    failed_restart_cnt= 0;
    work_cnt= 0;
    failed_work_cnt= 0;
    rework_cnt= 0;
    failed_rework_cnt= 0;
    interrupt_cnt= 0;
    fault_cnt= 0;
    node_failure_cnt= 0;
    total_repaired= 0;
    soft_reboot_success_cnt= 0;
    soft_reboot_failure_cnt= 0;

    rnd_gen_cnt= 0;
    rnd_prob_cnt= 0;
    calls_rMPI= 0;
    read_input_cnt= 0;
    read_input_accepted= 0;

}  /* end of init_globals() */
