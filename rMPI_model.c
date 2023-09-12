/*
** $Id: rMPI_model.c,v 1.32 2011/01/24 00:07:19 rolf Exp $
**
** Rolf Riesen, September 2009 -- 2010, Sandia National Laboratories
** All major time values are stored in doubles and are in minutes
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
#include <sys/types.h>
#include <math.h>
#include <assert.h>

#include <avl.h>
#include "globals.h"
#include "rMPI_model.h"
#include "rnd.h"
#include "data_structs.h"
#include "input.h"


/* Local functions */
static int compare_nodes(const void *pa, const void *pb, void *param);


/* Define a struct to hold info about a node. Make sure it is a multiple of sizeof(double) */
typedef struct node_t   {
    int ID;		/* My ID */
    int partner;	/* Who is backup for this node? */
    int active;		/* Which node of this bundle is the active node? */
    int dead;		/* Is this node dead? */
    double tod;		/* Time of death */
    double rebirth;	/* Time of rebirth after death (successful soft error reboot) */
    double new_tod;	/* If reborn, when will it happen? */
} node_t;

static node_t *nodes;
static struct avl_table *avl_nodes;
static int read_input= FALSE;


static nodelist_t *next_phase_kills_start= NULL;
static nodelist_t *next_phase_kills_end= NULL;


/* Local function */
static void process_previous_phase(double elapsed_time, double previous_app_death,
		FILE *fp_ints, FILE *fp_faults);
static node_t *init_node_array(int num_bundles, int total_nodes, int verbose);
static int find_next_node_to_die(int tree_change);
static int soft_boot_node(int dead_node, float soft_reboot_success_rate,
		float soft_time_to_reboot, int hotswap);
static void next_phase_kills_add(int node);
static void wakeup_node(int node);
static int is_bundle_dead(int dead_node);
static int count_bundle_nodes(int bundle);
#undef LEGACY
#define LEGACY
#ifdef LEGACY
    static void sort_list(void);
#endif



/*
** The first time through rMPI() we need to initialize the random number
** generator, allocate memory for the nodes, and initialize them.
*/
void
rMPI_init(int num_bundles, int total_nodes, FILE *fp_input, int verbose)
{


    /* Allocate memory for the nodes and initialize it */
    nodes= init_node_array(num_bundles, total_nodes, verbose);
    if (init_input(fp_input, num_bundles))   {
	read_input= TRUE;
    }

}  /* end of rMPI_init() */



/*
** This function determines which nodes die and when. It returns the time the app
** dies next time.
*/
double
rMPI(int verbose, FILE *fp_ints, FILE *fp_faults, double elapsed_time,
	float soft_time_to_reboot, float soft_reboot_success_rate,
	int hotswap)
{

static double previous_app_death= 0.0;
double next_app_death;
int dead_node;
int wake;
int rc;


    calls_rMPI++;
    /*
    ** If we are reading the fault times from a file, make sure the fault times
    ** are ascending and return the next value.
    */
    if (read_input)   {
	static int first_time= TRUE;

	next_app_death= read_next(verbose);
	if (next_app_death < 0)   {
	    fprintf(stderr, "ERROR: Input file terminated early or has error!\n");
	    exit(8);
	}

	if (next_app_death < previous_app_death)   {
	    fprintf(stderr, "ERROR: Fault times read from input file are not ascending!\n");
	    exit(8);
	} else   {
	    if (first_time)   {
		first_time= FALSE;
		/*
		** Don't count the first fault. We'll read one extra fault after the app
		** has finished and count that instead.
		** The input file must span a time longer than the application run time.
		*/
	    } else   {
		fault_cnt++;
		node_failure_cnt++;
		total_repaired++;
		if (fp_faults)   {
		    fprintf(fp_faults, "%15.3f\n", next_app_death);
		}

		/* FIXME: Once we allow redundant nodes with an input fault file, this needs to move. */
		if (fp_ints)   {
		    fprintf(fp_ints, "%15.3f %d\n", previous_app_death, 1);
		}
	    }
	    previous_app_death= next_app_death;
	    return next_app_death;
	}
    }



    /* Process the faults that occured in the last phase. */
    process_previous_phase(elapsed_time, previous_app_death, fp_ints, fp_faults);


    /*
    ** All nodes are alive again. We want to figure out when the application dies
    ** the next time. The lowest bundle tod may not be it, since a soft reboot
    ** may delay the app death.
    ** With soft reboots, things could run for a very long time. We need to know
    ** when the first bundle truly dies, even with soft reboots.
    */
    wake= 0;
    while (TRUE)   {
	dead_node= find_next_node_to_die(wake);
	wake= soft_boot_node(dead_node, soft_reboot_success_rate, soft_time_to_reboot, hotswap);
	next_app_death= nodes[dead_node].tod;
	rc= is_bundle_dead(dead_node);
	wakeup_node(wake);
	if (rc)   {
	    break;
	}
    }
    if (verbose > 4)   {
	fprintf(stderr, "# rMPI        Application dies at time %12.1f\" Time since last death %12.1f\"\n",
	    previous_app_death + next_app_death, next_app_death);
    }

    assert(next_app_death >= previous_app_death);
    previous_app_death= next_app_death;

    return next_app_death;

}  /* end of rMPI() */



/*
** This gets called from app_model() to find out how many nodes have died
** during the previous phase.
*/
int
count_dead_nodes(double elapsed_time, FILE *fp_faults)
{

int dead_nodes;
nodelist_t *list;
nodelist_t *next;


    dead_nodes= fault_cnt;
    list= next_phase_kills_start;
    while (list)   {
        if (nodes[list->node].tod <= elapsed_time)   {
            fault_cnt++;
            node_failure_cnt++;
	    if (fp_faults)   {
		fprintf(fp_faults, "%15.3f\n", nodes[list->node].tod);
	    }
        }

	next= list->next;
	free(list);
	list= next;
    }
    next_phase_kills_start= NULL;
    next_phase_kills_end= NULL;
    
    return fault_cnt - dead_nodes;

}  /* end of count_dead_nodes() */



/*
** -----------------------------------------------------------------------------
** Local functions
** -----------------------------------------------------------------------------
*/



/*
** During the previous phase a bunch of nodes died. This function marks and counts them,
** then rejuvenates them by assigning those nodes a new tod.
*/
static void
process_previous_phase(double elapsed_time, double previous_app_death,
	FILE *fp_ints, FILE *fp_faults)
{

int dead_nodes;
static int first_time= TRUE;
nodelist_t *list;
nodelist_t *next;
node_t *current;
node_t *old;
node_t **new;


    /* We should not come in here before the next app interrupt! */
    /* FIXME: Sometimes this triggers because of RAS delay which we do not take into consideration yet */
    assert(elapsed_time >= previous_app_death);


    if (first_time)   {
	first_time= FALSE;
    } else   {
	/*
	** Since last time nodes might have died that did not cause an
	** application interrupt. Sweep through and mark all failed
	** nodes.
	*/
	dead_nodes= fault_cnt;
#ifdef LEGACY
	sort_list();
#endif
	list= next_phase_kills_start;

	/* At least one node must have died or we wouldn't be here */
	assert(list);
	while (list)   {
	    assert(nodes[list->node].dead);
	    node_failure_cnt++;
	    if (fp_faults)   {
		fprintf(fp_faults, "%15.3f\n", nodes[list->node].tod);
	    }
	    fault_cnt++;

	    current= &(nodes[list->node]);
	    old= avl_delete(avl_nodes, current);
	    assert(old);

	    /* Only reset tod for failed nodes */
	    do   {
		/* See if we can insert it (must have unique tod) */
		current->tod= next_node_failure(elapsed_time);
		new= (node_t **)avl_probe(avl_nodes, current);
	    } while ((*new)->ID != list->node);

	    current->dead= FALSE;
	    total_repaired++;

	    next= list->next;
	    free(list);
	    list= next;
	}
	next_phase_kills_start= NULL;
	next_phase_kills_end= NULL;

	/* There should always be at least one dead node */
	assert(fault_cnt - dead_nodes);

	if (fp_ints)   {
	    fprintf(fp_ints, "%15.3f %d\n", previous_app_death, fault_cnt - dead_nodes);
	}
    }

}  /* end of process_previous_phase() */



static int
find_next_node_to_die(int tree_change)
{

node_t *current;
static struct avl_traverser traverser;


    if (tree_change >= 0)   {
	/* The tree has changed and the traverser is no longer valid */
	current= avl_t_first(&traverser, avl_nodes);
	assert(current);
    } else   {
	/* Pick up from where we left off the last time */
	current= avl_t_next(&traverser);
    }

    /* Now find the node with the lowest tod.  */
    while (current->dead == TRUE)   {
	/* Reject it, if it is already dead */
	current= avl_t_next(&traverser);
	assert(current);
    }

    return current->ID;

}  /* end of find_next_node() */



/*
** See if we can reboot this node. If so, return this node's ID.
** If not, return -1.
*/
static int
soft_boot_node(int dead_node, float soft_reboot_success_rate, float soft_time_to_reboot, int hotswap)
{

int bundle_cnt;
int rc;
node_t test;


    rc= -1;
    if (soft_reboot_success_rate >= 0.0)   {
	bundle_cnt= count_bundle_nodes(nodes[dead_node].active);
	if (bundle_cnt > 1)   {
	    /* Maybe */
	    if (rnd_probability() <= soft_reboot_success_rate)   {
		/*
		** Yes. During the time between original tod and tod + soft_time_to_reboot,
		** this node is in coma and could cause bundle failure.
		*/
		nodes[dead_node].rebirth= nodes[dead_node].tod + soft_time_to_reboot;

		/*
		** We have to make sure that we'll be able to insert the new tod.
		** It has to be unique in the search tree. That's why the following
		** is in a while loop.
		**
		** If we gave the node a new TOD like this:
		**     test.tod= next_node_failure(nodes[dead_node].rebirth);
		** that would work for exponential distribution, but not Weibull.
		** We are not givig this node a new life! So, we need to start at
		** time 0 for Weibull.
		** We have three options:
		** 1). Start life from now. That's like a hotswap with a new node
		** 2). Start at time 0, and if the new TOD is less than elapsed
		**     time, say that the node died during re-integration.
		** 3). Keep trying until we get a time that is beyond current
		**     and continue.
		*/
		do   {
		    if (hotswap)   {
			test.tod= next_node_failure(nodes[dead_node].rebirth);
		    } else   {
			test.tod= next_node_failure(0.0);
		    }
		} while (avl_find(avl_nodes, &test) != NULL);

		if (test.tod <= nodes[dead_node].rebirth)   {
		    /* Didn't make it */
		    soft_reboot_failure_cnt++;
		    next_phase_kills_add(dead_node);
		} else   {
		    /* Found one that works. (Still counts as a fault, though) */
		    nodes[dead_node].new_tod= test.tod;
		    rc= dead_node;
		    soft_reboot_success_cnt++;
		}
		fault_cnt++;
	    } else   {
		soft_reboot_failure_cnt++;
		next_phase_kills_add(dead_node);
	    }
	} else   {
	    /* No redundant nodes in this bundle */
	    next_phase_kills_add(dead_node);
	}
    } else   {
	/* Soft reboots not enabled */
	next_phase_kills_add(dead_node);
    }

    /* Mark it dead. It is in coma or dead for good. */
    nodes[dead_node].dead= TRUE;
    return rc;

}  /* end of soft_boot_node() */


static void
next_phase_kills_add(int node)
{
    nodelist_add(&next_phase_kills_start, &next_phase_kills_end, node);
}  /* end of next_phase_kills_add() */



/*
** If this is a valid node, remove its entry from the avl tree,
** update it, and reinsert it.
*/
static void
wakeup_node(int node)
{

node_t *current;
#ifndef NDEBUG
    node_t **new;
#endif


    if (node < 0)   {
	return;
    }

    /*
    ** We have to remove this node from the search tree and reinsert it,
    ** so that it will be in the correct position with its new tod
    */
    current= avl_delete(avl_nodes, &(nodes[node]));
    assert(current);

    /* Re-init node */
    nodes[node].dead= FALSE;
    nodes[node].rebirth= -1;
    nodes[node].tod= nodes[node].new_tod;
    nodes[node].new_tod= -1;

    /*
    ** There is a small chance that insertion will fail. We checked in soft_boot_node()
    ** to make sure that new_tod is unique, but we are not checking all the new_tod.
    ** So, in the meantime, a second identical new_tod might have been inserted in the
    ** search tree.
    ** UPDATE: We're doing one node at a time, so this cannot happen!
    **         Treat this as an assertion within NDEBUG
    */
    new= (node_t **)avl_probe(avl_nodes, &(nodes[node]));
#ifndef NDEBUG
    if ((*new)->ID != node)   {
	fprintf(stderr, "Attempting to insert non-unique tod into search tree!\n");
	exit(9);
    }
#endif

}  /* end of wakeup_node() */



/*
** At the tod of dead_node, is this bundle dead?
*/
static int
is_bundle_dead(int dead_node)
{

double t;
int follow;


    /*
    ** We need at least one node at time t in this bundle to be alive.
    ** Some nodes in this bundle may be in coma for a while. Check for
    ** that too.
    */
    t= nodes[dead_node].tod;
    follow= nodes[dead_node].active;
    do   {
	if ((nodes[follow].tod > t) ||
		((nodes[follow].new_tod > t) && (nodes[follow].rebirth < t)))   {
	    /* We are alive! */
	    return FALSE;
	}
	follow= nodes[follow].partner;
    } while (follow >= 0);

    return TRUE;

}  /* end of is_bundle_dead() */



/*
** The array nodes[] contains a list of all nodes present in the system.
** The first "num_bundle" nodes are the active nodes. Their "partner"
** links point to other nodes in this bundle.
** This function follows these links and counts how many nodes it finds
** in this bundle.
*/
static int
count_bundle_nodes(int bundle)
{

int follow;
int bundle_cnt;


    follow= bundle;
    bundle_cnt= 0;
    do   {
	bundle_cnt++;
	follow= nodes[follow].partner;
    } while (follow >= 0);

    return bundle_cnt;

}  /* end of count_bundle_nodes() */



/*
** We need a function to comapre the tod of nodes
*/
static int 
compare_nodes(const void *pa, const void *pb, void *param) 
{

const node_t *a= pa;
const node_t *b= pb;
void *unused;

    unused= param;
    if (a->tod < b->tod)   {
	return -1;
    } else if (a->tod > b->tod)   {
	return +1;
    } else   {
	return 0;
    }

}  /* end of compare_nodes() */



/*
** Allocate the node structures and fill them with default values.
** This gets called only once. alloc_node is freed at the end of
** the program automatically.
*/
static node_t *
init_node_array(int num_bundles, int total_nodes, int verbose)
{

int i;
int active_node;
node_t *alloc_nodes;
node_t **current;


    avl_nodes= avl_create(compare_nodes, NULL, NULL);
    alloc_nodes= (node_t *)malloc(total_nodes * sizeof(node_t));
    if (alloc_nodes == NULL)   {
	fprintf(stderr, "Out of memory!\n");
	exit(10);
    }

    /* Initialize the active nodes */
    for (i= 0; i < num_bundles; i++)   {
	alloc_nodes[i].ID= i;
	alloc_nodes[i].partner= -1;
	alloc_nodes[i].active= i;	/* We are the active node */
	alloc_nodes[i].rebirth= -1.0;	/* No rebirth */
	alloc_nodes[i].new_tod= -1.0;
	alloc_nodes[i].dead= FALSE;

	/* tod of each node in the tree has to be unique */
	do   {
	    alloc_nodes[i].tod= next_node_failure(0.0);
	    current= (node_t **)avl_probe(avl_nodes, &(alloc_nodes[i]));
	} while ((*current)->ID != i);
    }

    /* Assign redundant nodes to active nodes in round robin fashion */
    active_node= 0;
    for (i= num_bundles; i < total_nodes; i++)   {
	if (alloc_nodes[active_node % num_bundles].partner >= 0)   {
	    /*
	    ** This active node already has a redundant node assigned
	    ** Assign a redundant-redundant node etc.
	    */
	    /* FIXME: It would be nice to support this */
	    fprintf(stderr, "# rMPI        More redundant nodes than active nodes not supported yet!\n");
	    exit(10);
	} else   {
	    alloc_nodes[active_node % num_bundles].partner= i;	/* Tell the active node where we are */
	    alloc_nodes[i].ID= i;
	    alloc_nodes[i].active= active_node % num_bundles;	/* Who is our active node? */
	    alloc_nodes[i].partner= -1;
	    alloc_nodes[i].rebirth= -1.0;			/* No rebirth */
	    alloc_nodes[i].new_tod= -1.0;
	    alloc_nodes[i].dead= FALSE;

	    /* tod of each node in the tree has to be unique */
	    do   {
		alloc_nodes[i].tod= next_node_failure(0.0);
		current= (node_t **)avl_probe(avl_nodes, &(alloc_nodes[i]));
	    } while ((*current)->ID != i);

	    if (verbose > 0)   {
		fprintf(stderr, "# rMPI        Active node %5d has node %5d as redundant\n", active_node % num_bundles, i);
	    }
	}
	active_node++;
    }

#ifndef NDEBUG
    if (avl_count(avl_nodes) != (unsigned)total_nodes)   {
	fprintf(stderr, "%ld items in table does not match total number of nodes %d\n",
	    (long int)avl_count(avl_nodes), total_nodes);
	exit(10);
    }
#endif

    return alloc_nodes;

}  /* end of init_node_array() */



#ifdef LEGACY
/*
** Sort the list of nodes to be rejuvenated. The only reason to do this is to
** be compatible with an older version of app_model. This version assigns new
** tod in a different order to nodes. This changes the simulation, but does
** not make it incorrect. The only time this matters is when we use a fixed seed
** and make comparisions to old results.
** Since this is basically debugging, I make no attempt to speed this up.
*/
static void
sort_list(void)
{
nodelist_t *new_list_start;
nodelist_t *new_list_end;
nodelist_t *search;
nodelist_t *element;
nodelist_t *next;


    new_list_start= NULL;
    element= next_phase_kills_start;
    while (element)   {
	next= element->next;
	/* Insert this node into the new list where it belongs */
	if (!new_list_start)   {
	    /* First element in the new list */
	    new_list_start= element;
	    element->next= NULL;
	    new_list_end= element;
	} else   {
	    /* Traverse the list until just before a node that is bigger */
	    search= new_list_start;
	    while (search)   {
		if (search->node == element->node)   {
		    fprintf(stderr, "Can't have duplicate node IDs! %d\n", element->node);
		    exit(1);
		}

		if (search->node > element->node)   {
		    /* Our element goes at the start of the list! */
		    new_list_start= element;
		    element->next= search;
		    break;
		} else if (search->next == NULL)   {
		    /* We have reached the end of the list. Our element goes there. */
		    search->next= element;
		    element->next= NULL;
		    new_list_end= element;
		    break;
		} else   {
		    if (search->next->node > element->node)   {
			/* Insert our element here */
			element->next= search->next;
			search->next= element;
			break;
		    } else   {
			/* Keep going */
		    }
		}
		search= search->next;
	    }
	}

	element= next;
    }
    next_phase_kills_start= new_list_start;
    next_phase_kills_end= new_list_end;

}  /* end of sort_list() */
#endif  /* LEGACY */
