//
// Substring.h
//
// Substring: The substring fuzzy algorithm. Currently a rather slow, naive approach
//            that checks the substring against every word in the word db.
//            It does not generate a separate database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Substring.h,v 1.2 1999/09/10 17:22:25 ghutchis Exp $
//

#ifndef _Substring_h_
#define _Substring_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Substring : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Substring();
    virtual		~Substring();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex(Configuration &);

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


