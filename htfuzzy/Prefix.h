//
// Prefix.h
//
// Prefix: The prefix fuzzy algorithm. Performs a O(log n) search on for words
//         matching the *prefix* specified--thus significantly faster than a full
//         substring search.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Prefix.h,v 1.2 1999/09/10 17:22:24 ghutchis Exp $
//

#ifndef _Prefix_h_
#define _Prefix_h_

#include "Fuzzy.h"
#include "htfuzzy.h"

class Dictionary;
class String;
class List;


class Prefix : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Prefix();
    virtual		~Prefix();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex(Configuration &);

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


