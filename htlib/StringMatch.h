//
// StringMatch.h
//
// (c) 1995 Andrew Scherpbier <andrew@sdsu.edu>
//
// This class provides an interface to a fairly specialized string
// lookup facility.  It is intended to be used as a replace for any
// regualr expression matching when the pattern string is in the form:
//
//    <string1>|<string2>|<string3>|...
//
// Just like regular expression routines, the pattern needs to be
// compiled before it can be used.  This is done using the Pattern()
// member function.  Once the pattern has been compiled, the member
// function Find() can be used to search for the pattern in a string.
// If a string has been found, the "which" and "length" parameters
// will be set to the string index and string length respectively.
// (The string index is counted starting from 0) The return value of
// Find() is the position at which the string was found or -1 if no
// strings could be found.  If a case insensitive match needs to be
// performed, call the IgnoreCase() member function before calling
// Pattern().  This function will setup a character translation table
// which will convert all uppercase characters to lowercase.  If some
// other translation is required, the TranslationTable() member
// function can be called to provide a custom table.  This table needs
// to be 256 characters.
// 
// $Id: StringMatch.h,v 1.2.2.1 1999/09/01 21:00:19 grdetil Exp $
//
// $Log: StringMatch.h,v $
// Revision 1.2.2.1  1999/09/01 21:00:19  grdetil
// Add a StringMatch::IgnorePunct() method, which allows matches to skip over
// valid punctuation. Use it to highlight matching words in excerpts regardless
// of punctuation, and don't add short or bad words to logicalPattern.
//
// Revision 1.2  1999/01/21 03:41:09  ghutchis
// Add default parameter sep = '|'.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _StringMatch_h_
#define _StringMatch_h_

#include <Object.h>


class StringMatch : public Object
{
public:
    //
    // Construction/Destruction
    //
    StringMatch();
    ~StringMatch();

    //
    // Set the pattern to search for.  If given as a string needs to
    // be in the form <string1>|<string2>|...  If in the form of a
    // List, it should be a list of String objects.
    //
    void		Pattern(char *pattern, char sep = '|');

    //
    // Search for any of the strings in the pattern in the given
    // string The return value is the offset in the source a pattern
    // was found.  In this case, the which variable will be set to the
    // index of the pattern string and length will be set to the
    // length of that pattern string.  If none of the pattern strings
    // could be found, the return value will be -1
    //
    int			FindFirst(char *string, int &which, int &length);
    int			FindFirst(char *string);
	
    int			FindFirstWord(char *string, int &which, int &length);
    int			FindFirstWord(char *string);
	
    //
    // If you are interested in matching instead of searching, use
    // the following.  Same parameters except that the return value will
    // be 1 if there was a match, 0 if there was not.
    //
    int			Compare(char *string, int &which, int &length);
    int			Compare(char *string);

    int			CompareWord(char *string, int &which, int &length);
    int			CompareWord(char *string);
    
    //
    // Provide a character translation table which will be applied to
    // both the pattern and the input string.  This table should be an
    // array of 256 characters.  If is the caller's responsibility to
    // manage this table's allocation.  The table should remain valid
    // until this object has been destroyed.
    //
    void		TranslationTable(char *table);
	
    //
    // Build a local translation table which maps all uppercase
    // characters to lowercase
    //
    void		IgnoreCase();

    //
    // Build a local translation table which ignores all given punctuation
    // characters
    //
    void		IgnorePunct(char *punct = NULL);

    //
    // Determine if there is a pattern associated with this Match object.
    //
    int			hasPattern()		{return table[0] != 0;}
	
protected:
    int			*table[256];
    unsigned char	*trans;
    int			local_alloc;
};

#endif


