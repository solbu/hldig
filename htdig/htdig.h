//
// htdig.h
//
// $Id: htdig.h,v 1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: htdig.h,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#ifndef _htdig_h_
#define _htdig_h_

#include <Configuration.h>
#include <List.h>
#include "DocumentDB.h"
#include <StringMatch.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <stdio.h>
#include <htconfig.h>

extern Configuration	config;
extern int				debug;
extern DocumentDB		docs;
extern StringMatch		limits;
extern StringMatch		excludes;
extern FILE				*urls_seen;
extern FILE				*images_seen;

extern void reportError(char *msg);

#ifndef TRUE
# define	TRUE				(1)
#endif
#ifndef FALSE
# define	FALSE				(0)
#endif

#endif


