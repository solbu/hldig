//
// SuffixEntry.cc
//
// SuffixEntry: Decode the suffix rules used in the ispell dictionary files
//              for the endings fuzzy DB.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: SuffixEntry.cc,v 1.3 2002/02/01 22:49:33 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "SuffixEntry.h"
#include "Endings.h"


//*****************************************************************************
// SuffixEntry::SuffixEntry()
//
SuffixEntry::SuffixEntry(char *str)
{
    parse(str);
}


//*****************************************************************************
// SuffixEntry::~SuffixEntry()
//
SuffixEntry::~SuffixEntry()
{
}


//*****************************************************************************
// void SuffixEntry::parse(char *str)
//   Parse a string in the format <expr> '>' <rule> into ourselves.
//
void
SuffixEntry::parse(char *str)
{
    String	temp = 0;
    
    while (*str == ' ' || *str == '\t')
	str++;

    temp = "^.*";
    while (*str != '>')
    {
	if (*str != ' ' && *str != '\t')
	    temp << *str;
	str++;
    }
    temp << "$";
    while (*str == ' ' || *str == '\t' || *str == '>')
	str++;

    Endings::mungeWord(temp, expression);
    
    temp = 0;
    while (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r' && *str)
    {
	temp << *str;
	str++;
    }
    Endings::mungeWord(temp, rule);
}


