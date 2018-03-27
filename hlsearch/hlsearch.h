//
// hlsearch.h
//
// hlsearch: The main search CGI. Parses the CGI input, reads the config files
//           and calls the necessary code to put together the result lists
//           and the final display.
//
// Part of the hl://Dig package   <https://solbu.github.io/hldig>
// Copyright (c) 2017 The hl://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//  hl://Dig is a fork of ht://Dig <https://sourceforge.net/projects/htdig/>
//

#ifndef _hlsearch_h_
#define _hlsearch_h_

#include "List.h"
#include "StringList.h"
#include "Dictionary.h"
#include "DocumentRef.h"
#include "Database.h"
#include "good_strtok.h"
#include "DocumentDB.h"
#include "htString.h"
#include "HtConfiguration.h"
#include "ResultMatch.h"
#include "ResultList.h"
#include "HtWordReference.h"
#include "StringMatch.h"
#include "defaults.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif

extern int n_matches;
extern int do_and;
extern int do_short;
extern StringList fields;

#ifndef _MSC_VER                /* _WIN32 */
extern StringMatch limit_to;
#endif

extern StringMatch URLimage;
extern List URLimageList;
extern StringMatch wm;
extern Database *dbf;
extern String logicalWords;
extern String originalWords;
extern int debug;
extern StringList collectionList;

#endif
