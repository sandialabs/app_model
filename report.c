/*
** $Id: report.c,v 1.20 2010/12/21 23:32:32 rolf Exp $
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
#include "report.h"
#include "timing.h"
#include "globals.h"



void
report_results(double work_time, double elapsed_time, double calculated_sys_mtbf,
	double calculated_app_mtbf, int display_perf_info, double model_time, double daly,
	FILE *fp_input, double calculated_fpi)
{

double total_percent= 0.0;
double total_elapsed_time= 0.0;
float offset;
int num_faults;


    /* Report results */
    printf("\n");
    printf("SIMULATION\n");
    printf("  Application completed  %12.2f hours of work (%5.2f%% of work to be done)\n",
	total_work_time / 60.0, 100.0 / work_time * total_work_time);

    printf("  Elapsed time           %12.2f hours (Overhead is %5.2f%%)\n", elapsed_time / 60.0,
	(100.0 / work_time * elapsed_time) - 100.0);

    total_restart_time= total_restart_time + wasted_restart_time;
    printf("  Total restart time     %12.2f hours      (%6.2f%%)\n", total_restart_time / 60.0,
	100.0 / elapsed_time * total_restart_time);
    total_percent= total_percent + 100.0 / elapsed_time * total_restart_time;
    total_elapsed_time= total_elapsed_time + total_restart_time;

    total_rework_time= total_rework_time + wasted_rework_time;
    printf("  Total rework time      %12.2f hours      (%6.2f%%)\n", total_rework_time / 60.0,
	100.0 / elapsed_time * total_rework_time);
    total_percent= total_percent + 100.0 / elapsed_time * total_rework_time;
    total_elapsed_time= total_elapsed_time + total_rework_time;

    total_work_time= total_work_time + wasted_work_time;
    printf("  Total work time        %12.2f hours      (%6.2f%%)\n", total_work_time / 60.0,
	100.0 / elapsed_time * total_work_time);
    total_percent= total_percent + 100.0 / elapsed_time * total_work_time;
    total_elapsed_time= total_elapsed_time + total_work_time;

    total_checkpoint_time= total_checkpoint_time + wasted_checkpoint_time;
    printf("  Total checkpoint time  %12.2f hours      (%6.2f%%)\n", total_checkpoint_time / 60.0,
	100.0 / elapsed_time * total_checkpoint_time);
    total_percent= total_percent + 100.0 / elapsed_time * total_checkpoint_time;
    total_elapsed_time= total_elapsed_time + total_checkpoint_time;

    printf("  Total RAS delay        %12.2f hours      (%6.2f%%)\n", total_ras_delay / 60.0,
	100.0 / elapsed_time * total_ras_delay);
    total_percent= total_percent + 100.0 / elapsed_time * total_ras_delay;
    total_elapsed_time= total_elapsed_time + total_ras_delay;

    printf("    -----------------------------------------------------\n");
    printf("    Totals               %12.2f hours      (%6.2f%%)", total_elapsed_time / 60.0,
	total_percent);

    if ((total_percent <= 99.9) || (total_percent >= 100.1))   {
	printf("   ERROR: Percentages do not add up\n");
    } else   {
	printf("\n");
    }

    printf("\n");

#if REPORT_WASTED_SEPERATLY
    printf("  Wasted restart time    %12.2f hours\n", wasted_restart_time / 60.0);
    printf("  Wasted rework time     %12.2f hours\n", wasted_rework_time / 60.0);
    printf("  Wasted work time       %12.2f hours\n", wasted_work_time / 60.0);
    printf("  Wasted checkpoint time %12.2f hours\n", wasted_checkpoint_time / 60.0);

    printf("\n");
#endif

    /* We didn't run until the next interrupt */
    printf("  Number of restarts:        %5d    Failed:      %5d\n", restart_cnt,
	failed_restart_cnt);
    printf("  Number of rework:          %5d    Failed:      %5d\n", rework_cnt,
	failed_rework_cnt);
    printf("  Number of work segments:   %5d    Failed:      %5d\n", work_cnt,
	failed_work_cnt);
    printf("  Number of checkpoints:     %5d    Failed:      %5d\n", checkpoint_cnt,
	failed_checkpoint_cnt);

    num_faults= failed_restart_cnt + failed_rework_cnt + failed_work_cnt + failed_checkpoint_cnt;
    printf("    ----------------------------------------------------\n");
    printf("                                      Fails:       %5d\n", num_faults);
    printf("                                      Interrupts:  %5d\n", interrupt_cnt);
    if (num_faults != interrupt_cnt)   {
	printf("   ERROR: Num failures should equal num interrupts\n");
    }

    printf("\n");
    printf("  Faults:                 %8d\n", fault_cnt);
    printf("  Failed nodes:           %8d    Repaired:   %6d", node_failure_cnt, total_repaired);
    if (node_failure_cnt != total_repaired)   {
	printf(" (%d nodes to be repaired after app completion)\n", node_failure_cnt - total_repaired);
    } else   {
	printf("\n");
    }

    if ((soft_reboot_success_cnt + soft_reboot_failure_cnt) > 0)   {
	printf("  Successful soft reboots:  %6d    Failed:     %6d (%.2f%%)\n",
	    soft_reboot_success_cnt, soft_reboot_failure_cnt,
	    100.0 / (soft_reboot_success_cnt + soft_reboot_failure_cnt) * soft_reboot_success_cnt);
    } else   {
	printf("  Successful soft reboots:  %6d    Failed:     %6d (%.2f%%)\n",
	    0, 0, 0.0);
    }

    if (interrupt_cnt > 0)   {
	printf("  Avg faults per int:     %8.3f", (double)fault_cnt / interrupt_cnt);
	offset= 100.0 / calculated_fpi * (fault_cnt / interrupt_cnt);
	if (offset < 100.0)   {
	    printf(",   %.2f%% under calculated %.2f\n", 100.0 - offset, calculated_fpi);
	} else   {
	    printf(",   %.2f%% over calculated %.2f\n", offset - 100.0, calculated_fpi);
	}
    } else   {
	printf("  Avg faults per int:    >%8.3f\n", (double)fault_cnt);
    }

    if (fault_cnt > 0)   {
	printf("  System MTBF            %12.2f hours (%.3f minutes)",
	    (total_elapsed_time / fault_cnt) / 60.0,
	    total_elapsed_time / fault_cnt);

	if (fp_input)   {
	    printf("\n");
	} else   {
	    offset= 100.0 / calculated_sys_mtbf * (total_elapsed_time / fault_cnt);
	    if (offset < 100.0)   {
		printf(", %.2f%% under calculated %.2f hours\n",
		    100.0 - offset, calculated_sys_mtbf / 60.0);
	    } else   {
		printf(", %.2f%% over calculated %.2f hours\n",
		    offset - 100.0, calculated_sys_mtbf / 60.0);
	    }
	}
    } else   {
	printf("  Measured system MTBF higher than running time.\n");
    }

    if (interrupt_cnt > 0)   {
	printf("  App. MTBI              %12.2f hours (%.3f minutes)",
	    (elapsed_time / interrupt_cnt) / 60.0, elapsed_time / interrupt_cnt);

	if (fp_input)   {
	    printf("\n");
	} else   {
	    offset= 100.0 / calculated_app_mtbf * (total_elapsed_time / interrupt_cnt);
	    if (offset < 100.0)   {
		printf(", %.2f%% under calculated %.2f hours\n",
		    100.0 - offset, calculated_app_mtbf / 60.0);
	    } else   {
		printf(", %.2f%% over calculated %.2f hours\n",
		    offset - 100.0, calculated_app_mtbf / 60.0);
	    }
	}
    } else   {
	printf("  Measured application MTBF larger than running time\n");
    }

    if (fp_input)   {
	printf("  Cannot model elapsed time (Daly) [Or could I after the run?]\n");
    } else   {
	offset= 100.0 / total_elapsed_time * daly;
	if (offset < 100.0)   {
	    printf("  Modeled elapsed time   %12.2f hours (%.3f minutes), %.2f%%, under simulated %.2f hours\n",
		daly / 60.0, daly, 100.0 - offset, total_elapsed_time / 60.0);
	} else   {
	    printf("  Modeled elapsed time   %12.2f hours (%.3f minutes), %.2f%%, over simulated %.2f hours\n",
		daly / 60.0, daly, offset - 100.0, total_elapsed_time / 60.0);
	}
    }

    if (display_perf_info)   {
	printf("\n");
	printf("PROGRAM PERFORMANCE INFORMATION:\n");
	printf("  Generated %d random numbers and %d random probabilities\n",
	    rnd_gen_cnt, rnd_prob_cnt);
	printf("  Calls to rMPI() %d\n", calls_rMPI);
	if (read_input_cnt * read_input_accepted > 0.0)   {
	    printf("  Read %d faults from input file, accepted %d (%.2f%%)\n",
		read_input_cnt, read_input_accepted,
		100.0 / read_input_cnt * read_input_accepted);
	} else   {
	    printf("  Read %d faults from input file, accepted %d (%.2f%%)\n",
		read_input_cnt, read_input_accepted, 0.0);
	}
	printf("  Time to model this application: %s\n", disp_time(model_time));
    }

}  /* end of report_results() */
