//
// htfuzzy.h
//
// htfuzzy: Create one or more ``fuzzy'' indexes into the main word database.
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
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htfuzzy.h,v 1.9 2003/06/24 20:06:19 nealr Exp $
//

#ifndef _htfuzzy_h_
#define _htfuzzy_h_

#include "htconfig.h"
#include "HtConfiguration.h"
#include "HtWordList.h"

#include <stdlib.h>

#ifndef _MSC_VER //_WIN32
#include <unistd.h>
#endif

#include <fstream.h>
#include <stdio.h>

extern int		debug;

extern void reportError(char *msg);

#endif


