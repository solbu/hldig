//
// HtRegex.h
//
// HtRegex: A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegex.h,v 1.3 1999/09/08 14:42:29 loic Exp $
//
//
#ifndef	_HtRegex_h_
#define	_HtRegex_h_

#include "Object.h"
#include "StringList.h"
#include <sys/types.h>
#include <regex.h>
#include <fstream.h>

class HtRegex : public Object
{
public:
    //
    // Construction/Destruction
    //
    HtRegex();
    HtRegex(char *str);
    ~HtRegex();

    //
    // Methods
    //
    void	set(char *str);
    void	setEscaped(StringList &list);
    int		match(char *str, int nullmatch, int nullstr);

protected:
    int			compiled;
    regex_t		re;
};

#endif
