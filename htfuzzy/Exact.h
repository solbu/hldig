//
// Exact.h
//
// Exact: The exact-match "fuzzy" matching.  Simply returns the word (minus punctuation)
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Exact.h,v 1.3 1999/09/24 10:29:01 loic Exp $
//

#ifndef _Exact_h_
#define _Exact_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Exact : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Exact(const Configuration& config_arg);
    virtual		~Exact();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex();

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


