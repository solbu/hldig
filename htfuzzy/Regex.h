//
// Regex.h
//
// Regex: A fuzzy to match input regex against the word database.
//        Based on the substring fuzzy
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Regex.h,v 1.3.2.1 1999/12/07 19:54:11 bosc Exp $
//

#ifndef _Regex_h_
#define _Regex_h_

#include "Fuzzy.h"
#include "HtRegex.h"

class Dictionary;
class String;
class List;


class Regex : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Regex(const HtConfiguration& config_arg);
    virtual		~Regex();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


