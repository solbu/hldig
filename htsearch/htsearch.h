//
// htsearch.h
//
// $Id: htsearch.h,v 1.4 1999/03/21 15:23:56 hp Exp $
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
#include "WordReference.h"
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


