//
// Speling.h
//
// Speling: (sic) Performs elementary (one-off) spelling correction for ht://Dig
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Speling.h,v 1.3.2.2 2000/10/20 03:40:56 ghutchis Exp $
//

#ifndef _Speling_h_
#define _Speling_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Speling : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Speling(const HtConfiguration& config_arg);
    virtual		~Speling();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


