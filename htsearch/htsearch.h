/***************************************************************************
                          new_htsearch.h  -  description
                             -------------------
    begin                : Fri Oct 8 1999
    copyright            : (C) 1999 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//
// htsearch.h
//
// $Id: htsearch.h,v 1.8 1999/10/15 03:33:50 jtillman Exp $
//

#ifndef _htsearch_h_
#define _htsearch_h_

#include "List.h"
#include "StringList.h"
#include "Dictionary.h"
#include "DocumentRef.h"
#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include "Database.h"
#include "good_strtok.h"
#include "DocumentDB.h"
#include "htString.h"
#include "Configuration.h"
#include "ResultMatch.h"
#include "ResultList.h"
#include "HtWordReference.h"
#include "StringMatch.h"
#include "defaults.h"

extern int		n_matches;
extern int		do_and;
extern int		do_short;
extern StringList	fields;
extern StringMatch	limit_to;
extern StringMatch	URLimage;
extern List		URLimageList;
extern StringMatch	wm;
extern Database		*dbf;
extern String		logicalWords;
extern String		originalWords;
extern int              debug;

#endif






