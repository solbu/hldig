//
// htdig.h
//
// $Id: htdig.h,v 1.3 1998/10/21 16:34:19 bergolth Exp $
//
// $Log: htdig.h,v $
// Revision 1.3  1998/10/21 16:34:19  bergolth
// Added translation of server names. Additional limiting after normalization of the URL.
//
// Revision 1.2  1997/07/03 17:44:38  turtle
// Added support for virtual hosts
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
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
extern int		debug;
extern DocumentDB	docs;
extern StringMatch	limits;
extern StringMatch	limitsn;
extern StringMatch	excludes;
extern FILE		*urls_seen;
extern FILE		*images_seen;

extern void reportError(char *msg);

#ifndef TRUE
# define	TRUE				(1)
#endif
#ifndef FALSE
# define	FALSE				(0)
#endif

#endif


