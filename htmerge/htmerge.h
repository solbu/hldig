//
// htmerge.h
//
// $Id: htmerge.h,v 1.5 1999/06/01 01:55:58 ghutchis Exp $
//
//
//
#ifndef _htmerge_h_
#define _htmerge_h_

#include "defaults.h"
#include "WordRecord.h"
#include "DocumentDB.h"
#include "Database.h"
#include "HtURLCodec.h"
#include "htString.h"
#include "good_strtok.h"
#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


extern char		**array;
extern int		n_array_elements;
extern Dictionary	discard_list;
extern int		verbose;
extern int		stats;
extern Configuration	merge_config;


void mergeDB();
void mergeWords(char *wordtmp, char *wordfile);
void convertDocs(char *docs, char *index, char *excerpt);
void sort(char *);
void reportError(char *msg);

#endif


