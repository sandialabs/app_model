/*
** $Id: main.c,v 1.33 2011/01/24 00:07:19 rolf Exp $
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
#include <stdlib.h>
#include <string.h>		/* For strcmp(), strerror() */
#include <math.h>		/* For sqrt() */
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <gsl/gsl_sf_gamma.h>

#include "globals.h"
#include "app.h"
#include "report.h"
#include "rnd.h"
#include "rMPI_model.h"
#include "timing.h"


/*
** Change this when the output or the calculation changes
*/
#define VERSION			"1.005"


/*
** Defaults for command line parameters
*/
#define DEFAULT_NODE_MTBF	(43800)	/* 5 years */
#define DEFAULT_NUM_BUNDLES	(512)
#define DEFAULT_NUM_REDUNDANT	(0)
#define DEFAULT_CHECKPOINT_TIME	(5.0)
#define DEFAULT_RESTART_TIME	(10.0)
#define DEFAULT_WORK_TIME	(168.0)
#define DEFAULT_RAS_DELAY	(0.0)
#define DEFAULT_SHAPE		(0.5)
#define DEFAULT_SCALE		DEFAULT_NODE_MTBF



/*
** Local functions
*/
static void calc(double *tau, double *calculated_sys_mtbf, double *calculated_app_mtbf, 
	double *calculated_fpi, int num_bundles,
	int num_redundant, double node_mtbf, double checkpoint_time);
static void usage(int argc, char *argv[]);
static void banner(int argc, char *argv[], int num_bundles, int num_redundant, double checkpoint_time,
		double restart_time, double work_time, double tau, int tau_given, double node_mtbf,
		double calculated_sys_mtbf, int sys_mtbf_given, double calculated_app_mtbf, int app_mtbf_given,
		int default_seed, rnd_t rnd, double scale, double shape, char *fname_interrupts, char *fname_faults, double ras_delay,
		float soft_reboot_success_rate, float soft_time_to_reboot, FILE *fp_input,
		char *fname_input, double calculated_fpi);


static struct option long_options[]=   {
    /* name, has arg, flag, val */
    {"work_time", 1, NULL, 'w'},
    {"num_bundles", 1, NULL, 'n'},
    {"redundant", 1, NULL, 'r'},
    {"verbose", 0, NULL, 'v'},
    {"performance", 0, NULL, 'p'},
    {"distribution", 1, NULL, 1100},
    {"seed", 0, NULL, 's'},
    {"checkpoint_time", 1, NULL, 'c'},
    {"restart_time", 1, NULL, 'R'},
    {"tau", 1, NULL, 't'},
    {"mtbf_node", 1, NULL, 'm'},
    {"mtbf_sys", 1, NULL, 1003},
    {"mtbf_app", 1, NULL, 'a'},
    {"delay_ras", 1, NULL, 'd'},
    {"finterrupts", 1, NULL, 1000},
    {"ffaults", 1, NULL, 1001},
    {"soft_reboot", 1, NULL, 1002},
    {"help", 0, NULL, 1004},
    {"input", 1, NULL, 1005},
    {"shape", 1, NULL, 1006},
    {"scale", 1, NULL, 1007},
    {"hotswap", 0, NULL, 1008},
    {0, 0, 0, 0}
};


int
main(int argc, char *argv[])
{

int option_index = 0;
int ch, error;
char *endptr;
int verbose;

int num_bundles, num_redundant;
double checkpoint_time;
double restart_time;
double work_time;
double tau;
double node_mtbf;
double calculated_sys_mtbf;
double calculated_app_mtbf;
int default_seed;
double dist_shape;
double dist_scale;
rnd_t rnd;
char *fname_interrupts;
char *fname_faults;
char *fname_input;
FILE *fp_ints;
FILE *fp_faults;
FILE *fp_input;
double elapsed;
double ras_delay;
float soft_reboot_success_rate, soft_time_to_reboot;
int display_perf_info;
double t0, t1;
int tau_given;
int app_mtbf_given, sys_mtbf_given;
int help;
double daly;
double calculated_fpi;
int hotswap;



    /* Defaults */
    error= FALSE;
    verbose= 0;
    num_bundles= DEFAULT_NUM_BUNDLES;
    num_redundant= DEFAULT_NUM_REDUNDANT;
    node_mtbf= DEFAULT_NODE_MTBF;
    default_seed= FALSE;
    checkpoint_time= DEFAULT_CHECKPOINT_TIME;
    restart_time= DEFAULT_RESTART_TIME;
    work_time= DEFAULT_WORK_TIME;
    tau= -1.0;
    tau_given= FALSE;
    fname_interrupts= "";
    fname_faults= "";
    fname_input= "";
    rnd= RND_EXP;
    dist_shape= DEFAULT_SHAPE;
    dist_scale= DEFAULT_SCALE;
    ras_delay= DEFAULT_RAS_DELAY;
    soft_reboot_success_rate= -1.0;
    display_perf_info= FALSE;
    calculated_sys_mtbf= -1.0;
    calculated_app_mtbf= -1.0;
    sys_mtbf_given= FALSE;
    app_mtbf_given= FALSE;
    hotswap= FALSE;
    help= FALSE;


    /* check command line args */
    while (1)   {
	ch= getopt_long(argc, argv, "a:m:c:r:w:t:n:R:vsd:p", long_options, &option_index);
	if (ch == -1)   {
	    break;
	}

	switch (ch)   {
	    case 'a':
		calculated_app_mtbf= strtod(optarg, &endptr);
		if ((calculated_app_mtbf <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -mtbf_app %s\n", optarg);
		    error= TRUE;
		}
		calculated_app_mtbf= calculated_app_mtbf * 60.0;
		app_mtbf_given= TRUE;
		break;
	    case 'n':
		num_bundles= strtol(optarg, (char **)NULL, 0);
		if (num_bundles < 1)   {
		    fprintf(stderr, "-n %s must be > 0\n", optarg);
		    error= TRUE;
		}
		break;
	    case 'r':
		num_redundant= strtol(optarg, (char **)NULL, 0);
		if (num_redundant < 0)   {
		    fprintf(stderr, "-r %s must be >= 0\n", optarg);
		    error= TRUE;
		}
		break;
	    case 'p':
		display_perf_info= TRUE;
		break;
	    case 'v':
		verbose++;
		break;
	    case 1100:
		if ((strcmp(optarg, "g") == 0) || (strcmp(optarg, "gamma") == 0))   {
		    rnd= RND_GAMMA;
		} else
		if ((strcmp(optarg, "e") == 0) || (strcmp(optarg, "exp") == 0))   {
		    rnd= RND_EXP;
		} else
		if ((strcmp(optarg, "w") == 0) || (strcmp(optarg, "weibull") == 0))   {
		    rnd= RND_WEIBULL;
		} else   {
		    fprintf(stderr, "Unknown random distribution function: \"%s\"\n", optarg);
		    error= TRUE;
		}
		break;
	    case 's':
		default_seed= TRUE;
		break;
	    case 'c':
		checkpoint_time= strtod(optarg, &endptr);
		if ((checkpoint_time <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -c %s\n", optarg);
		    error= TRUE;
		}
		break;
	    case 'R':
		restart_time= strtod(optarg, &endptr);
		if ((restart_time <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -R %s\n", optarg);
		    error= TRUE;
		}
		break;
	    case 'w':
		work_time= strtod(optarg, &endptr);
		if ((work_time <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -w %s\n", optarg);
		    error= TRUE;
		}
		break;
	    case 't':
		tau= strtod(optarg, &endptr);
		if ((tau <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -t %s\n", optarg);
		    error= TRUE;
		}
		tau_given= TRUE;
		break;
	    case 'm':
		node_mtbf= strtod(optarg, &endptr);
		if ((node_mtbf <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -m %s\n", optarg);
		    error= TRUE;
		}
		break;
	    case 'd':
		ras_delay= strtod(optarg, &endptr);
		if ((ras_delay <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -d %s\n", optarg);
		    error= TRUE;
		}
		break;
	    case 1000:
		fname_interrupts= optarg;
		break;
	    case 1001:
		fname_faults= optarg;
		break;
	    case 1002:
		sscanf(optarg, "%f,%f", &soft_reboot_success_rate, &soft_time_to_reboot);
		if (soft_reboot_success_rate < 0.0 || soft_reboot_success_rate > 100.0)   {
		    fprintf(stderr, "Soft reboot success rate must be 0%% - 100%%. Got %f\n",
			soft_reboot_success_rate);
		    error= TRUE;
		}
		if (soft_time_to_reboot < 0.0)   {
		    fprintf(stderr, "Soft reboot time must be >= 0.0 Got %f\n", soft_time_to_reboot);
		    error= TRUE;
		}
		break;
	    case 1003:
		calculated_sys_mtbf= strtod(optarg, &endptr);
		if ((calculated_sys_mtbf <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid number for -mtbf_sys %s\n", optarg);
		    error= TRUE;
		}
		calculated_sys_mtbf= calculated_sys_mtbf * 60.0;
		sys_mtbf_given= TRUE;
		break;
	    case 1004:
		help= TRUE;
		break;
	    case 1005:
		fname_input= optarg;
		break;
	    case 1006:
		dist_shape= strtod(optarg, &endptr);
		if ((dist_shape <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid input for --shape %s. Must be > 0\n", optarg);
		    error= TRUE;
		}
		break;
	    case 1007:
		dist_scale= strtod(optarg, &endptr);
		if ((dist_scale <= 0.0) && (endptr == optarg))   {
		    fprintf(stderr, "Invalid input for --scale %s. Must be > 0\n", optarg);
		    error= TRUE;
		}
		break;
	    case 1008:
		hotswap= TRUE;
		break;
	    default:
		error= TRUE;
		break;
	}
    }

    if (optind < argc)   {
	error= TRUE;
	fprintf(stderr, "No additional arguments expected!\n");
    }

    if (error || help)   {
	usage(argc, argv);
	exit(1);
    }


    /*
    ** Open files if necessary
    */
    if (strcmp(fname_interrupts, "") == 0)   {
	/* Default */
	fp_ints= NULL;
    } else if (strcmp(fname_interrupts, "-") == 0)   {
	fp_ints= stdout;
    } else   {
	fp_ints= fopen(fname_interrupts, "w");
	if (fp_ints == NULL)   {
	    fprintf(stderr, "Could not open output file \"%s\": %s\n", fname_interrupts, strerror(errno));
	    exit(2);
	}
    }


    if (strcmp(fname_faults, "") == 0)   {
	/* Default */
	fp_faults= NULL;
    } else if (strcmp(fname_faults, "-") == 0)   {
	fp_faults= stdout;
    } else   {
	fp_faults= fopen(fname_faults, "w");
	if (fp_faults == NULL)   {
	    fprintf(stderr, "Could not open output file \"%s\": %s\n", fname_faults, strerror(errno));
	    exit(2);
	}
    }


    if (strcmp(fname_input, "") == 0)   {
	/* Default */
	fp_input= NULL;
    } else if (strcmp(fname_input, "-") == 0)   {
	fp_input= stdin;
    } else if (strcmp(fname_input, "") != 0)   {
	fp_input= fopen(fname_input, "r");
	if (fp_input == NULL)   {
	    fprintf(stderr, "Could not open input file \"%s\": %s\n", fname_input, strerror(errno));
	    exit(2);
	}
    }


    /* Some sanity checks */
    if (fp_input && (num_redundant > 0))   {
	fprintf(stderr, "Redundant nodes not supported when reading faults from a file.\n");
	exit(3);
    }


    /* Convert work time to minutes like everything else */
    work_time= 60.0 * work_time;
    node_mtbf= 60.0 * node_mtbf;
    dist_scale= 60.0 * dist_scale;

    init_rnd(rnd, node_mtbf, default_seed, dist_shape, dist_scale);
    init_globals();

    calc(&tau, &calculated_sys_mtbf, &calculated_app_mtbf, &calculated_fpi, num_bundles,
		num_redundant, node_mtbf, checkpoint_time);

    banner(argc, argv, num_bundles, num_redundant, checkpoint_time, restart_time, work_time,
		tau, tau_given, node_mtbf, calculated_sys_mtbf, sys_mtbf_given, calculated_app_mtbf,
		app_mtbf_given, default_seed, rnd, dist_scale, dist_shape, fname_interrupts,
		fname_faults, ras_delay, soft_reboot_success_rate, soft_time_to_reboot, fp_input,
		fname_input, calculated_fpi);

    rMPI_init(num_bundles, num_bundles + num_redundant, fp_input, verbose);

    t0= get_clock_value();
    elapsed= app_model(verbose, tau, checkpoint_time, restart_time, work_time, ras_delay,
		fp_ints, fp_faults, soft_time_to_reboot, soft_reboot_success_rate, hotswap);
    t1= get_clock_value();

    /* At this point we're one over */
    interrupt_cnt--;

    /*
    ** Without redundant nodes, the number of faults and interrupts must be the same.
    ** However, that is not true if ras_delay > 0, because that can consume more faults
    ** that there are interrupts.
    */
    assert((num_redundant != 0) || (interrupt_cnt == fault_cnt) || (ras_delay > 0.0));

    /* This is eq 20 from Daly:04:higher */
    daly= calculated_app_mtbf *
	    exp(restart_time / calculated_app_mtbf) *
	    (exp((tau + checkpoint_time) / calculated_app_mtbf) - 1.0) *
	    (work_time / tau);

    report_results(work_time, elapsed, calculated_sys_mtbf, calculated_app_mtbf,
		display_perf_info, t1 - t0, daly, fp_input, calculated_fpi);

    if (fp_ints)	fclose(fp_ints);
    if (fp_faults)	fclose(fp_faults);
    if (fp_input)	fclose(fp_input);

    return 0;

}  /* end of main() */



#define pi      (3.14159265358979323846264338327950288)
static double
Qm2(int n)
{
    return sqrt(pi * n / 2.0) + (2.0/3.0);
}  /* end of Qm2() */



static void
calc(double *tau, double *calculated_sys_mtbf, double *calculated_app_mtbf,
	double *calculated_fpi, int num_bundles,
	int num_redundant, double node_mtbf, double checkpoint_time)
{

#define pi	(3.14159265358979323846264338327950288)
float p;
float r_none, r_double;


    /*
    ** Calculate the system and application MTBI. We use it to calculate the
    ** optimal checkpoint interval (tau) using Daly's equation.
    */

    /* Percentage of redundant nodes */
    p= (float)num_redundant / (float)num_bundles;

    if (*calculated_sys_mtbf < 0.0)   {
	/* Not provided by user */
	*calculated_sys_mtbf= node_mtbf / (num_bundles * (1.0 + p));
    }


    /* Calculate the MTBF of the non-redundant portion of the system */
    if (num_redundant >= num_bundles)   {
	/* Each bundle has redundant nodes */
	r_none= 0.0;
    } else   {
	/* Some bundles do not have redundant nodes */
	r_none= node_mtbf / (num_bundles - num_redundant);
    }

    /* Calculate the MTBF for the bundles with 2 redundant nodes */
    if (num_redundant > 0)   {
	r_double= (node_mtbf / (2.0 * num_redundant)) * Qm2(2.0 * num_redundant);
    } else   {
	/* no bundles with redundant ndoes */
	r_double= 0.0;
    }

    if (*calculated_app_mtbf < 0.0)   {
	/* Unless supplied by user */
	if (num_redundant >= num_bundles)   {
	    /* All bundles have redundant nodes */
	    *calculated_app_mtbf= r_double;
	} else if (num_redundant > 0)   {
	    /* Only some bundles have redundant nodes */
	    /* FIXME: This calculation is wrong, I think. */
	    *calculated_app_mtbf= 1.0 / ((1.0 / r_none) + (1.0 / r_double));
	} else   {
	    /* No bundles have redundant nodes */
	    *calculated_app_mtbf= r_none;
	}
    }



    /*
    ** calculate optimal checkpoint interval (tau) using Daly's equation,
    ** unless the user supplied a specific tau.
    */
    if (*tau < 0.0)   {
	if (checkpoint_time >= (2.0 * *calculated_app_mtbf))   {
	    *tau= *calculated_app_mtbf;
	} else   {
	    *tau= sqrt(2.0 * checkpoint_time * *calculated_app_mtbf) *
		(1.0 + 
		 (1.0 / 3.0) * sqrt(checkpoint_time / (2.0 * *calculated_app_mtbf)) +
		 (checkpoint_time / (9.0 * 2.0 * *calculated_app_mtbf))
		) - checkpoint_time;
	}

	if (*tau < 0.0)   {
	    printf("   ERROR: tau (checkpoint time) is negative!\n");
	    exit(4);
	}
    }

    if (num_bundles == num_redundant)   {
	*calculated_fpi= Qm2(2.0 * num_bundles);
    } else if (num_redundant == 0)   {
	*calculated_fpi= 1.0;
    } else   {
	/* FIXME: Don't know how to do that yet */
	*calculated_fpi= -3.0;
    }

}  /* end of calc() */



static void
usage(int argc, char *argv[])
{

int i;


    fprintf(stderr, "Command line \"");
    for (i= 0; i < argc; i++)   {
	fprintf(stderr, "%s", argv[i]);
	if (i < (argc - 1))   {
	    fprintf(stderr, " ");
	}
    }
    fprintf(stderr, "\"\n");

    fprintf(stderr, "Usage: %s [-n num] [-r redundant] [-c checkpoint] [-R restart] "
	"[-w work] [-t tau] [-m mtbf]\n"
	"\t\t[-d delay] [--fi fi_name] [--ff ff_name] [--input ff_input]\n"
	"\t\t[-v {-v}] [-p] [-s] [--distrib dist] [--scale a] [--shape b] [--soft_reboot <success rate>,<reboot time>]\n"
	"\t\t[--mtbf_sys mtbf_sys] [-a mtbi_app] [--daly] [--help]\n", argv[0]);

    fprintf(stderr, "    -n num                       Number of bundles (active nodes) to simulate. (Default %d)\n",
	DEFAULT_NUM_BUNDLES);
    fprintf(stderr, "    -r redundant                 Number of redundant nodes. (Default %d)\n",
	DEFAULT_NUM_REDUNDANT);
    fprintf(stderr, "    -c checkpoint    (minutes)   Time to write a checkpoint. (Default %g minutes)\n",
	DEFAULT_CHECKPOINT_TIME);
    fprintf(stderr, "    -R restart       (minutes)   Time to relaunch and read a checkpoint. (Default %g minutes)\n",
	DEFAULT_RESTART_TIME);
    fprintf(stderr, "    -w work          (hours)     Time to do the application work. (Default %g hours)\n",
	DEFAULT_WORK_TIME);
    fprintf(stderr, "    -t tau           (minutes)   Time between checkpoints. (tau_opt calculated if not specified)\n");
    fprintf(stderr, "    -m mtbf          (hours)     Node MTBF. (Default %.0f hours)\n",
	(float)DEFAULT_NODE_MTBF);
    fprintf(stderr, "    --mtbf_sys mtbf_sys  (h)     Set system MTBF. (Only used for result comparisons)\n");
    fprintf(stderr, "    -a mtbi_app      (hours)     Application MTBI to calculate tau_opt. (Overrides node mtbf)\n");
    fprintf(stderr, "    -d delay         (minutes)   Multiple faults within this time count as one. (Default %g minutes)\n",
	DEFAULT_RAS_DELAY);
    fprintf(stderr, "    --fi fi_name                 File name to write restart (interrupt) times. (Default /dev/null)\n");
    fprintf(stderr, "    --ff ff_name                 File name to write fault times. (Default /dev/null)\n");
    fprintf(stderr, "    --input ff_input             File name to read fault times from. Prevents fault generation by sim.\n");
    fprintf(stderr, "    --distrib dist               Random distribution function: exp (default), gamma, weibull\n");
    fprintf(stderr, "    --scale a        (hours)     Scale parameter for Weibull and gamma distribution. (Default %.3f)\n", (float)DEFAULT_SCALE);
    fprintf(stderr, "    --shape b                    Shape parameter for Weibull and gamma distribution. (Default %.3f)\n", DEFAULT_SHAPE);
    fprintf(stderr, "    -v                           Increase verbosity. Option may be repeated.\n");
    fprintf(stderr, "    -s                           Use fixed seed for random number generator (repeat runs)\n");
    fprintf(stderr, "    --soft_reboot success rate,  Percentage of nodes that can be brought back to life doing a reboot (0 - 1.0)\n");
    fprintf(stderr, "                  reboot time    Nodes become available again after this many minutes\n");
    fprintf(stderr, "    -p                           Display simulation performance data\n");
    fprintf(stderr, "    --help                       This message\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "app_model, version %s - a program that mimics a parallel\n", VERSION);
    fprintf(stderr, "application's checkpoint/restart behavior on a system with\n");
    fprintf(stderr, "redundant compute nodes.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Copyright 2009 Sandia Corporation. Under the terms of Contract\n");
    fprintf(stderr, "DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government\n");
    fprintf(stderr, "retains certain rights in this software.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "This program has been released into the public domain using the\n");
    fprintf(stderr, "GNU General Public License Version 3.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Author: Rolf Riesen <rolf@sandia.gov>\n");

}  /* end of usage() */



static void
banner(
    int argc,
    char *argv[],
    int num_bundles,
    int num_redundant,
    double checkpoint_time,
    double restart_time,
    double work_time,
    double tau,
    int tau_given,
    double node_mtbf,
    double calculated_sys_mtbf,
    int sys_mtbf_given,
    double calculated_app_mtbf,
    int app_mtbf_given,
    int default_seed,
    rnd_t rnd,
    double scale,
    double shape,
    char *fname_interrupts,
    char *fname_faults,
    double ras_delay,
    float soft_reboot_success_rate,
    float soft_time_to_reboot,
    FILE *fp_input,
    char *fname_input,
    double calculated_fpi)
{

int i;
char str[1024];


    printf("Version %s\n", VERSION);
    printf("Command line \"");
    for (i= 0; i < argc; i++)   {
	printf("%s", argv[i]);
	if (i < (argc - 1))   {
	    printf(" ");
	}
    }
    printf("\"\n");

    printf("PARAMETERS\n");
    printf("  Active nodes           %12d\n", num_bundles);
    printf("  Redundant nodes        %12d\n", num_redundant);
    printf("  Total nodes            %12d\n", num_bundles + num_redundant);
    printf("  Checkpoint duration    %12.2f minutes\n", checkpoint_time);
    printf("  Restart duration       %12.2f minutes\n", restart_time);
    printf("  Work to be done        %12.2f hours\n", work_time / 60.0);

    if (fp_input)   {
	printf("  Cannot calculate node MTBF\n");
    } else   {
	printf("  Node MTBF              %12.2f hours\n", node_mtbf / 60.0);
    }

    if (sys_mtbf_given)   {
	printf("  System MTBF            %12.2f hours (%.3f minutes)\n", calculated_sys_mtbf / 60.0, calculated_sys_mtbf);
    }

    if (app_mtbf_given)   {
	printf("  Application MTBI       %12.2f hours (%.3f minutes)\n", calculated_app_mtbf / 60.0, calculated_app_mtbf);
    }

    if (tau_given)   {
	printf("  Checkpoint interval    %12.2f hours (%.3f minutes)\n", tau / 60.0, tau);
    }

    printf("  File for interrupt times            \"%s\"\n", fname_interrupts);
    printf("  File for fault times                \"%s\"\n", fname_faults);
    if (fp_input != NULL)   {
	printf("  File to read fault times from       \"%s\"\n", fname_input);
    }

    if (default_seed)   {
	printf("  Seed for pseudo random generator    fixed\n");
    } else   {
	printf("  Seed for pseudo random generator    random\n");
    }

    if (fp_input)   {
	printf("  Fault distribution:               %13s\n", "Input file");
    } else   {
	switch (rnd)   {
	    case RND_EXP:
		printf("  Fault distribution:                 exponential\n");
		break;
	    case RND_GAMMA:
		printf("  Fault distribution:                 gamma (scale %.3f, shape %.3f)\n",
		    scale, shape);
		break;
	    case RND_WEIBULL:
		if ((1.0 + 1.0 / shape) > GSL_SF_GAMMA_XMAX)   {
		    fprintf(stderr, "Shape parameter %g is too large for gamma function!\n",
			shape);
		    exit(1);
		}

		printf("  Fault distribution:                 weibull (scale %.3f, shape %.3f, gamma %.3f)\n",
		    scale, shape, gsl_sf_gamma(1.0 + 1.0 / shape));
		break;
	    default:
		fprintf(stderr, "Unknown random distribution!\n");
		exit(1);
		break;
	}
    }

    /* Faults within this interval count as one */
    printf("  RAS delay              %12.2f minutes\n", ras_delay);

    if (soft_reboot_success_rate < 0.0)   {
	printf("  Soft reboot time                    not used\n");
    } else   {
	printf("  Soft reboot time       %12.2f minutes\n", soft_time_to_reboot);
	printf("  Soft reboot success    %12.2f%%\n", soft_reboot_success_rate);
    }



    printf("\n");
    printf("CALCULATED\n");
    if (!sys_mtbf_given)   {
	if (fp_input)   {
	    printf("  Cannot calculate system MTBF\n");
	} else   {
	    printf("  System MTBF            %12.2f hours (%.3f minutes)\n", calculated_sys_mtbf / 60.0, calculated_sys_mtbf);
	}
    }
    if (!app_mtbf_given)   {
	if (fp_input)   {
	    printf("  Cannot estimate application MTBI\n");
	} else   {
	    printf("  Application MTBI       %12.2f hours (%.3f minutes)\n", calculated_app_mtbf / 60.0, calculated_app_mtbf);
	}
    }
    if (!tau_given)   {
	if (fp_input)   {
	    printf("  Checkpoint interval    %12.2f hours (%.3f minutes) (Default)\n", tau / 60.0, tau);
	} else   {
	    printf("  Checkpoint interval    %12.2f hours (%.3f minutes)\n", tau / 60.0, tau);
	}
    }
    if ((num_bundles == num_redundant) || (num_redundant == 0))   {
	printf("  Faults/interrupt       %12.2f\n", calculated_fpi);
    } else   {
	sprintf(str, "1...???");
	printf("  Faults/interrupt       %12s\n", str);
    }


    if ((restart_time + checkpoint_time) > calculated_sys_mtbf)   {
	if ((restart_time + checkpoint_time) > (10.0 * calculated_sys_mtbf))   {
	    printf("WARNING: checkpoint + restart time > system MTBF! This WILL take extremly long!\n");
	} else   {
	    printf("WARNING: checkpoint + restart time > system MTBF! This may take quite a while.\n");
	}
    }

    if (checkpoint_time >= work_time)   {
	printf("WARNING: checkpoint time >= work time! Daly model will be wrong.\n");
    }


}  /* end of banner() */
