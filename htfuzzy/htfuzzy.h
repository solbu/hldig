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
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htfuzzy.h,v 1.4 1999/09/24 10:29:02 loic Exp $
//

#ifndef _htfuzzy_h_
#define _htfuzzy_h_

#include "htconfig.h"
#include "Configuration.h"
#include "WordList.h"

#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <stdio.h>

extern int		debug;

extern void reportError(char *msg);

#endif


