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
// $Id: HtWordType.cc,v 1.8 2000/02/19 05:29:03 ghutchis Exp $
//

#include "HtWordType.h"
#include "WordType.h"

int HtIsWordChar(char c)          { return WordType::Instance()->IsChar(c); }
int HtIsStrictWordChar(char c)	  { return WordType::Instance()->IsStrictChar(c); }
int HtWordNormalize(String &w)	  { return WordType::Instance()->Normalize(w); }
int HtStripPunctuation(String &w) { return WordType::Instance()->StripPunctuation(w); }
