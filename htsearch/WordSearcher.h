#ifndef _WordSearcher_h_
#define _WordSearcher_h_

//
// WordSearcher.h
//
// WordSearcher: a simple word database readonly-access wrapper
//               generates ResultLists for the Query framework.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordSearcher.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#if HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "htString.h"
#include "HtWordList.h"

class ResultList;

class WordSearcher
{
public:
	// constructor
	WordSearcher(const String &filename);

	// fetch results for one exact word
	ResultList *Search(const String &word);

private:
	// word is to be ignored
	bool IsIgnore(const String &word);

	// fetch results in database
	ResultList *Fetch(const String &word);

	// the database wrapper
	HtWordList references;
};
	
	
#endif /* _WordSearcher_h_ */
