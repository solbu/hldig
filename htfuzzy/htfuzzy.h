//
// htfuzzy.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htfuzzy.h,v 1.2 1999/09/10 01:37:39 ghutchis Exp $
//
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

extern Configuration	config;
extern int		debug;

extern void reportError(char *msg);

#endif


