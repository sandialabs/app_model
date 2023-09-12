/*
** $Id: globals.h,v 1.10 2010/12/21 23:32:32 rolf Exp $
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
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define FALSE			(0)
#define TRUE			(1)

void init_globals(void);

/* Turn assertions on (NDEBUG) or off (!NDEBUG) */
#define NDEBUG
#undef NDEBUG

/* Some global time keepers */
extern double total_restart_time;
extern double total_rework_time;
extern double total_work_time;
extern double total_checkpoint_time;
extern double total_ras_delay;

extern double wasted_restart_time;
extern double wasted_rework_time;
extern double wasted_work_time;
extern double wasted_checkpoint_time;

/* Global counters */
extern int checkpoint_cnt, failed_checkpoint_cnt;
extern int restart_cnt, failed_restart_cnt;
extern int rework_cnt, failed_rework_cnt;
extern int work_cnt, failed_work_cnt;
extern int interrupt_cnt;
extern int fault_cnt;
extern int node_failure_cnt;
extern int total_repaired;
extern int soft_reboot_success_cnt;
extern int soft_reboot_failure_cnt;
extern int rnd_gen_cnt;
extern int rnd_prob_cnt;

extern int calls_rMPI;
extern int read_input_cnt;
extern int read_input_accepted;


#endif /* _GLOBALS_H_ */
