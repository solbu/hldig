//
// SuffixEntry.cc
//
// Implementation of SuffixEntry
//
// $Log: SuffixEntry.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: SuffixEntry.cc,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $";
#endif

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


