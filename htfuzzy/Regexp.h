//
// Regexp.h
//
// Regexp: A fuzzy to match input regex against the word database.
//        Based on the substring fuzzy
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Regexp.h,v 1.3 2003/06/24 20:06:19 nealr Exp $
//

#ifndef _Regexp_h_
#define _Regexp_h_

#include "Fuzzy.h"
#include "HtRegex.h"

class Dictionary;
class String;
class List;


class Regexp : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Regexp(const HtConfiguration& config_arg);
    virtual		~Regexp();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


