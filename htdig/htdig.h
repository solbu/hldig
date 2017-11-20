//
// htdig.h
//
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htdig.h,v 1.16 2004/05/28 13:15:16 lha Exp $
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

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

#include <stdio.h>

#include "gettext.h"
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

extern int debug;
extern DocumentDB docs;
extern HtRegexList limits;
extern HtRegexList limitsn;
extern HtRegexList excludes;
extern HtRegexList badquerystr;
extern FILE *urls_seen;
extern FILE *images_seen;

extern void reportError (char *msg);

#endif
