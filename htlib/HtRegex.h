//
// HtRegex.h
//
// A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegex.h,v 1.1 1999/04/30 23:48:07 ghutchis Exp $
//
//
#ifndef	_HtRegex_h_
#define	_HtRegex_h_

#include <Object.h>
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
    int		match(char *str, int nullmatch, int nullstr);

protected:
    int			compiled;
    regex_t		re;
};

#endif
