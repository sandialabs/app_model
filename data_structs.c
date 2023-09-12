/*
** $Id: data_structs.c,v 1.3 2010/12/21 23:32:32 rolf Exp $
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
#include <stdlib.h>

#include "data_structs.h"



void
nodelist_add(nodelist_t **list_start, nodelist_t **list_end, int node)
{

nodelist_t *new;


    /* Get a new entry */
    new= (nodelist_t *)malloc(sizeof(nodelist_t));
    if (!new)   {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }

    new->node= node;
    new->next= NULL;

    if (*list_end)   {
	(*list_end)->next= new;
    }

    if (!(*list_start))   {
	*list_start= new;
    }

    *list_end= new;

}  /* end of nodelist_add() */
