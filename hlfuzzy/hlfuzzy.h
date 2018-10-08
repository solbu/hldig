//
// hlfuzzy.h
//
// hlfuzzy: Create one or more ``fuzzy'' indexes into the main word database.
//          These indexes can be used by htsearch to perform a search that uses
//          other algorithms than exact word match.
//
//  This program is meant to be run after htmerge has created the word
//  database.
//
//  For each fuzzy algorithm, there will be a separate database.  Each
//  database is simply a mapping from the fuzzy key to a list of words
//  in the main word database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: hlfuzzy.h,v 1.12 2004/05/28 13:15:20 lha Exp $
//

#ifndef _hlfuzzy_h_
#define _hlfuzzy_h_

#include "hlconfig.h"
#include "HtConfiguration.h"
#include "HtWordList.h"

#include <stdlib.h>

#ifndef _MSC_VER /* _WIN32 */
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

extern int    debug;

extern void reportError(const char *msg);

#endif


