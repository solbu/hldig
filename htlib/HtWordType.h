//
// HtWordType.h
//
//  functions for determining valid words/characters
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordType.h,v 1.8.2.3 2000/02/22 23:15:49 grdetil Exp $
//
#ifndef _HtWordType_h
#define _HtWordType_h

#include "htString.h"

extern int HtIsWordChar(char c);
extern int HtIsStrictWordChar(char c);
extern int HtWordNormalize(String &w);
extern int HtStripPunctuation(String &w);

// Like strtok(), but using our rules for word separation.
extern char *HtWordToken(char *s);

#endif /* _HtWordType_h */
