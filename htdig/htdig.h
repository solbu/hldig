//
// htdig.h
//
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htdig.h,v 1.9 1999/09/11 05:03:51 ghutchis Exp $
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


