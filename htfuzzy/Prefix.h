//
// Prefix.h
//
// Prefix: The prefix fuzzy algorithm. Performs a O(log n) search on for words
//         matching the *prefix* specified--thus significantly faster than a full
//         substring search.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Prefix.h,v 1.6 2003/06/24 20:06:19 nealr Exp $
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
    Prefix(const HtConfiguration& config_arg);
    virtual		~Prefix();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


