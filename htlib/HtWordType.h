//
// HtWordType.h
//
//  functions for determining valid words/characters
// 
//  StringMatch previously included WordType.h, this made
//  htlib dependent of htword, which is not acceptable for
//  a library...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordType.h,v 1.8.2.1 2000/01/03 10:02:05 bosc Exp $
//
#ifndef _HtWordType_h
#define _HtWordType_h


extern int HtIsWordChar(char c);
extern int HtIsStrictWordChar(char c);
extern int HtWordNormalize(String &w);
extern int HtStripPunctuation(String &w);

#endif
