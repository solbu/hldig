//
// htdig.h
//
// $Id: htdig.h,v 1.5 1999/01/27 00:34:51 ghutchis Exp $
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
extern StringMatch      badquerystr;
extern FILE		*urls_seen;
extern FILE		*images_seen;

extern void reportError(char *msg);

#endif


