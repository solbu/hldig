
//
// htdig.h
//
// $Id: htdig.h,v 1.6 1999/04/30 23:48:07 ghutchis Exp $
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
#ifdef REGEX
extern HtRegex		limits;
extern HtRegex		limitsn;
extern HtRegex		excludes;
#else
extern StringMatch	limits;
extern StringMatch	limitsn;
extern StringMatch	excludes;
#endif
extern StringMatch      badquerystr;
extern FILE		*urls_seen;
extern FILE		*images_seen;

extern void reportError(char *msg);

#endif


