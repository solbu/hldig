//
// Substring.h
//
// Substring: The substring fuzzy algorithm. Currently a rather slow, naive approach
//            that checks the substring against every word in the word db.
//            It does not generate a separate database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Substring.h,v 1.5 2002/02/01 22:49:33 ghutchis Exp $
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
    Substring(const HtConfiguration& config_arg);
    virtual		~Substring();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


