//
// htdig.h
//
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htdig.h,v 1.9.2.3 2001/02/11 23:08:29 ghutchis Exp $
//

#ifndef _htdig_h_
#define _htdig_h_

#include "HtConfiguration.h"
#include "List.h"
#include "DocumentDB.h"
#include "StringMatch.h"
#include "htconfig.h"
#include "HtRegexList.h"
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <stdio.h>

extern HtConfiguration	config;
extern int		debug;
extern DocumentDB	docs;
extern HtRegexList	limits;
extern HtRegexList	limitsn;
extern HtRegexList	excludes;
extern HtRegexList	badquerystr;
extern FILE		*urls_seen;
extern FILE		*images_seen;

extern void reportError(char *msg);

#endif


