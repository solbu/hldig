//
// htmerge.h
//
// $Id: htmerge.h,v 1.4 1999/01/21 13:41:22 ghutchis Exp $
//
// $Log: htmerge.h,v $
// Revision 1.4  1999/01/21 13:41:22  ghutchis
// Check HtURLCodec for errors.
//
// Revision 1.3  1999/01/09 20:17:07  ghutchis
// Declare new mergeDB code.
//
// Revision 1.2  1997/03/24 04:33:23  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _htmerge_h_
#define _htmerge_h_

#include <defaults.h>
#include <WordRecord.h>
#include <DocumentDB.h>
#include <htString.h>
#include <good_strtok.h>
#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <Database.h>
#include <ctype.h>
#include <string.h>
#include <HtURLCodec.h>


extern char			**array;
extern int			n_array_elements;
extern Dictionary	discard_list;
extern int			verbose;
extern int			stats;
extern Configuration	merge_config;


void mergeDB();
void mergeWords(char *wordtmp, char *wordfile);
void convertDocs(char *docs, char *docgdbm);
void sort(char *);
void reportError(char *msg);

#endif


