//
// htdig.h
//
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// $Id: htdig.h,v 1.8 1999/09/08 17:11:16 loic Exp $
//
//
#ifndef _htdig_h_
#define _htdig_h_

#include "Configuration.h"
#include "List.h"
#include "DocumentDB.h"
#include "StringMatch.h"
#include "htconfig.h"
#include "HtRegex.h"
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <stdio.h>

extern Configuration	config;
extern int		debug;
extern DocumentDB	docs;
extern HtRegex		limits;
extern HtRegex		limitsn;
extern HtRegex		excludes;
extern HtRegex		badquerystr;
extern FILE		*urls_seen;
extern FILE		*images_seen;

extern void reportError(char *msg);

#endif


