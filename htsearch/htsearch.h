//
// htsearch.h
//
// $Id: htsearch.h,v 1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: htsearch.h,v $
// Revision 1.1  1997/02/03 17:11:05  turtle
// Initial revision
//
// Revision 1.1  1996/01/03 19:01:40  turtle
// Before rewrite
//
//
#ifndef _htsearch_h_
#define _htsearch_h_

#include <List.h>
#include <StringList.h>
#include <Dictionary.h>
#include <DocumentRef.h>
#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <Database.h>
#include <good_strtok.h>
#include <DocumentDB.h>
#include <String.h>
#include <Configuration.h>
#include "ResultMatch.h"
#include "ResultList.h"
#include <WordReference.h>
#include <StringMatch.h>
#include <defaults.h>

extern int		n_matches;
extern int		do_and;
extern int		do_short;
extern StringList	fields;
extern StringMatch	limit_to;
extern StringMatch	URLimage;
extern List		URLimageList;
extern StringMatch	wm;
extern char		*valid_punctuation;
extern Database		*dbf;
extern String		logicalWords;
extern String		originalWords;


#endif


