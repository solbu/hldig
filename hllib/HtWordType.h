//
// HtWordType.h
//
//  functions for determining valid words/characters
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtWordType.h,v 1.12 2004/05/28 13:15:21 lha Exp $
//
#ifndef _HtWordType_h
#define _HtWordType_h

#include "htString.h"

extern int HtIsWordChar (char c);
extern int HtIsStrictWordChar (char c);
extern int HtWordNormalize (String & w);
extern int HtStripPunctuation (String & w);

// Like strtok(), but using our rules for word separation.
extern char *HtWordToken (char *s);

#endif /* _HtWordType_h */
