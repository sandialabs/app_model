/*
** $Id: data_structs.h,v 1.3 2010/12/21 23:32:32 rolf Exp $
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
#ifndef _DATA_STRUCTS_H_
#define _DATA_STRUCTS_H_

typedef struct nodelist_t   {
    int node;
    struct nodelist_t *next;
} nodelist_t;

void nodelist_add(nodelist_t **list_start, nodelist_t **list_end, int node);

#endif /* _DATA_STRUCTS_H_ */
