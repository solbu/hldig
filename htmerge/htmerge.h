//
// htmerge.h
//
// $Id: htmerge.h,v 1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: htmerge.h,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#ifndef _htmerge_h_
#define _htmerge_h_

#include <defaults.h>
#include <WordRecord.h>
#include <DocumentDB.h>
#include <String.h>
#include <good_strtok.h>
#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <Database.h>
#include <ctype.h>
#include <string.h>


extern char			**array;
extern int			n_array_elements;
extern Dictionary	discard_list;
extern int			verbose;
extern int			stats;

void mergeWords(char *wordtmp, char *wordfile);
void convertDocs(char *docs, char *docgdbm);
void sort(char *);
void reportError(char *msg);

#endif


