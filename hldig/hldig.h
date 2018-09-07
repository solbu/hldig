//
// hldig.h
//
// hldig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the hl://Dig package   <https://solbu.github.io/hldig/>
// Copyright (c) 2017 The hl://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//  hl://Dig is a fork of ht://Dig <https://sourceforge.net/projects/htdig/>
//

#ifndef _hldig_h_
#define _hldig_h_

#include "HtConfiguration.h"
#include "List.h"
#include "DocumentDB.h"
#include "StringMatch.h"
#include "hlconfig.h"
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

extern int debug;
extern DocumentDB docs;
extern HtRegexList limits;
extern HtRegexList limitsn;
extern HtRegexList excludes;
extern HtRegexList badquerystr;
extern FILE *urls_seen;
extern FILE *images_seen;

extern void reportError (const String& msg);

#endif
