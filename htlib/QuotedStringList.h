//
// QuotedStringList.h
//
// QuotedStringList: Fed with a string it will extract separator delimited
//                   words and store them in a list. The words may be 
//                   delimited by " or ', hence the name.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: QuotedStringList.h,v 1.3 1999/09/11 05:03:52 ghutchis Exp $
//

#ifndef _QuotedStringList_h_
#define _QuotedStringList_h_

#include "StringList.h"

class QuotedStringList : public StringList
{
public:
    //
    // Construction/Destruction
    //
    QuotedStringList();
    ~QuotedStringList();

    //
    // Creation of a String from a string or String
    //
    QuotedStringList(char *, char sep = '\t', int single = 0);
    QuotedStringList(String &, char sep = '\t', int single = 0);
    QuotedStringList(char *, char *sep, int single = 0);
    QuotedStringList(String &, char *sep, int single = 0);

    int			Create(char *, char sep = '\t', int single = 0);
    int			Create(String &, char sep = '\t', int single = 0);
    int			Create(char *, char *sep, int single = 0);
    int			Create(String &, char *sep, int single = 0);
private:
};

#endif


