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
// $Id: HtRegex.h,v 1.5.2.1 1999/12/02 02:41:24 ghutchis Exp $
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
    HtRegex(const char *str, int case_sensitive = 0);
    ~HtRegex();

    //
    // Methods for setting the pattern
    //
    void	set(const String& str, int case_sensitive = 0) { set(str.get(), case_sensitive); }
    void	set(const char *str, int case_sensitive = 0);
    void	setEscaped(StringList &list, int case_sensitive = 0);

    //
    // Methods for checking a match
    //
    int		match(const String& str, int nullmatch, int nullstr) { return match(str.get(), nullmatch, nullstr); }
    int		match(const char *str, int nullmatch, int nullstr);

protected:
    int			compiled;
    regex_t		re;
};

#endif
