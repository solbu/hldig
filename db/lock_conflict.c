/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: lock_conflict.c,v 1.1.2.2 2000/09/14 03:13:21 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"

/*
 * The conflict arrays are set up such that the row is the lock you
 * are holding and the column is the lock that is desired.
 */
const u_int8_t CDB_db_rw_conflicts[] = {
	/*		N   R   W */
	/*   N */	0,  0,  0,
	/*   R */	0,  0,  1,
	/*   W */	0,  1,  1
};

const u_int8_t CDB_db_riw_conflicts[] = {
	/*		N	S	X	IX	IS	SIX */
	/*   N */	0,	0,	0,	0,	0,	0,
	/*   S */	0,	0,	1,	1,	0,	1,
	/*   X */	1,	1,	1,	1,	1,	1,
	/*  IX */	0,	1,	1,	0,	0,	0,
	/*  IS */	0,	0,	1,	0,	0,	0,
	/* SIX */	0,	1,	1,	0,	0,	0
};
