//
// StringMatch.cc
//
// (c) 1995 Andrew Scherpbier <andrew@sdsu.edu>
//
// Implementation of StringMatch
//
//
#if RELEASE
static char RCSid[] = "$Id: StringMatch.cc,v 1.10 1999/08/25 21:50:17 grdetil Exp $";
#endif

#include "StringMatch.h"
#include <string.h>
#include <ctype.h>
#include <fstream.h>
#include "HtWordType.h"

//
// Entries in the state table can either be normal or final.
// Final states have an match index encoded in them.  This number
// is shifted left by INDEX_SHIFT bits.
//
#define	MATCH_INDEX_MASK	0xffff0000
#define	STATE_MASK		0x0000ffff
#define	INDEX_SHIFT		16

//*****************************************************************************
// StringMatch::StringMatch()
//
StringMatch::StringMatch()
{
    //
    // Clear out the state table pointers
    //
    for (int i = 0; i < 256; i++)
	table[i] = 0;
    local_alloc = 0;
    trans = 0;
}


//*****************************************************************************
// StringMatch::~StringMatch()
//
StringMatch::~StringMatch()
{
    for (int i = 0; i < 256; i++)
	delete [] table[i];
    if (local_alloc)
	delete [] trans;
}


//*****************************************************************************
// void StringMatch::Pattern(char *pattern)
//   Compile the given pattern into a state transition table
//
void
StringMatch::Pattern(char *pattern, char sep)
{
    if (!pattern || !*pattern)
    {
	//
	// No pattern to compile...
	//
	return;
    }

    //
    // Allocate enough space in the state table to hold the worst case
    // patterns...
    //
    int		n = strlen(pattern);

    // ...but since the state table does not need an extra state
    // for each string in the pattern, we can subtract the number
    // of separators.  Wins for small but numerous strings in
    // the pattern.
    char *tmpstr;
    for (tmpstr = pattern;
         (tmpstr = strchr(tmpstr, sep)) != NULL;
         tmpstr++)              // Pass the separator.
      n--;

    int		i;

    for (i = 0; i < 256; i++)
    {
	table[i] = new int[n];
	memset((unsigned char *) table[i], 0, n * sizeof(int));
    }
    for (i = 0; i < n; i++)
	table[0][i] = i;	// "no-op" states for null char, to be ignored

    //
    // Set up a standard case translation table if needed.
    //
    if (!trans)
    {
	trans = new unsigned char[256];
	for (i = 0; i < 256; i++)
	{
	    trans[i] = (unsigned char)i;
	}
	local_alloc = 1;
    }

    //
    // Go though each of the patterns and build entries in the table.
    //
    int			state = 0;
    int			totalStates = 0;
    unsigned char	previous = 0;
    int			previousState = 0;
    int			previousValue = 0;
    int			index = 1;
    unsigned char	chr;
    
    while ((unsigned char)*pattern)
    {
#if 0
	if (totalStates > n)
	{
	  cerr << "Fatal!  Miscalculation of number of states"
	       << endl;
	  exit (2);
	}
#endif

	chr = trans[(unsigned char)*pattern];
	if (chr == 0)
	{
	    pattern++;
	    continue;
	}
	if (chr == sep)
	{
	    //
	    // Next pattern
	    //
	    table[previous][previousState] =
		previousValue | (index << INDEX_SHIFT);
	    index++;
	    state = 0;
	    //	    totalStates--;
	}
	else
	{
	    previousValue = table[chr][state];
	    previousState = state;
	    if (previousValue)
	    {
		if (previousValue & MATCH_INDEX_MASK)
		{
		    if (previousValue & STATE_MASK)
		    {
			state = previousValue & STATE_MASK;
		    }
		    else
		    {
			table[chr][state] |= ++totalStates;
			state = totalStates;
		    }
		}
		else
		{
		    state = previousValue & STATE_MASK;
		}
	    }
	    else
	    {
		table[chr][state] = ++totalStates;
		state = totalStates;
	    }
	}
	previous = chr;
	pattern++;
    }
    table[previous][previousState] =
	previousValue | (index << INDEX_SHIFT);
}


//*****************************************************************************
// int StringMatch::FindFirst(char *string, int &which, int &length)
//   Attempt to find the first occurance of the previous compiled patterns.
//
int StringMatch::FindFirst(char *string, int &which, int &length)
{
    which = -1;
    length = -1;

    if (!table[0])
	return 0;

    int		state = 0, new_state = 0;
    int		pos = 0;
    int		start_pos = 0;

    while ((unsigned char)string[pos])
    {
	new_state = table[trans[(unsigned char)string[pos] & 0xff]][state];
	if (new_state)
	{
	    if (state == 0)
	    {
		//
		// Keep track of where we started comparing so that we can
		// come back to this point later if we didn't match anything
		//
		start_pos = pos;
	    }
	}
	else
	{
	    //
	    // We came back to 0 state.  This means we didn't match anything.
	    //
	    if (state)
	    {
		// But we may already have a match, and are just being greedy.
		if (which != -1)
		    return start_pos;
   
		pos = start_pos + 1;
	    }
	    else
		pos++;
	    state = 0;
	    continue;
	}
	state = new_state;
	if (state & MATCH_INDEX_MASK)
	{
	    //
	    // Matched one of the patterns.
	    // Determine which and return.
	    //
	    which = ((unsigned int) (state & MATCH_INDEX_MASK)
		     >> INDEX_SHIFT) - 1;
	    length = pos - start_pos + 1;
	    state &= STATE_MASK;

	    // Continue to find the longest, if there is one.
	    if (state == 0)
		return start_pos;
	}
	pos++;
    }

    // Maybe we were too greedy.
    if (which != -1)
	return start_pos;

    return -1;
}


//*****************************************************************************
// int StringMatch::Compare(char *string, int &which, int &length)
//
int StringMatch::Compare(char *string, int &which, int &length)
{
    which = -1;
    length = -1;

    if (!table[0])
	return 0;

    int		state = 0, new_state = 0;
    int		pos = 0;
    int		start_pos = 0;

    //
    // Skip to at least the start of a word.
    //
    while ((unsigned char)string[pos])
    {
	new_state = table[trans[string[pos]]][state];
	if (new_state)
	{
	    if (state == 0)
	    {
		start_pos = pos;
	    }
	}
	else
	{
	    // We may already have a match, and are just being greedy.
	    if (which != -1)
		return 1;
   
	    return 0;
	}
	state = new_state;
	if (state & MATCH_INDEX_MASK)
	{
	    //
	    // Matched one of the patterns.
	    //
	    which = ((unsigned int) (state & MATCH_INDEX_MASK)
		     >> INDEX_SHIFT) - 1;
	    length = pos - start_pos + 1;

	    // Continue to find the longest, if there is one.
	    state &= STATE_MASK;
	    if (state == 0)
		return 1;
	}
	pos++;
    }

    // Maybe we were too greedy.
    if (which != -1)
	return 1;

    return 0;
}


//*****************************************************************************
// int StringMatch::FindFirstWord(char *string)
//
int StringMatch::FindFirstWord(char *string)
{
    int	dummy;
    return FindFirstWord(string, dummy, dummy);
}


//*****************************************************************************
// int StringMatch::CompareWord(char *string)
//
int StringMatch::CompareWord(char *string)
{
    int	dummy;
    return CompareWord(string, dummy, dummy);
}


//*****************************************************************************
// int StringMatch::FindFirstWord(char *string, int &which, int &length)
//   Attempt to find the first occurance of the previous compiled patterns.
//
int StringMatch::FindFirstWord(char *string, int &which, int &length)
{
    which = -1;
    length = -1;

    int		state = 0, new_state = 0;
    int		pos = 0;
    int		start_pos = 0;
    int		is_word = 1;

    //
    // Skip to at least the start of a word.
    //
    while ((unsigned char)string[pos])
    {
	new_state = table[trans[(unsigned char)string[pos]]][state];
	if (new_state)
	{
	    if (state == 0)
	    {
		start_pos = pos;
	    }
	}
	else
	{
	    //
	    // We came back to 0 state.  This means we didn't match anything.
	    //
	    if (state)
	    {
		pos = start_pos + 1;
	    }
	    else
		pos++;
	    state = 0;
	    continue;
	}
	state = new_state;

	if (state & MATCH_INDEX_MASK)
	{
	    //
	    // Matched one of the patterns.
	    //
	    is_word = 1;
	    if (start_pos != 0)
	    {
		if (HtIsStrictWordChar((unsigned char)string[start_pos - 1]))
		    is_word = 0;
	    }
	    if (HtIsStrictWordChar((unsigned char)string[pos + 1]))
		is_word = 0;
	    if (is_word)
	    {
		//
		// Determine which and return.
		//
		which = ((unsigned int) (state & MATCH_INDEX_MASK)
			 >> INDEX_SHIFT) - 1;
		length = pos - start_pos + 1;
		return start_pos;
	    }
	    else
	    {
		//
		// Not at the end of word.  Continue searching.
		//
		if (state & STATE_MASK)
		{
		    state &= STATE_MASK;
		}
		else
		{
		    pos = start_pos + 1;
		    state = 0;
		}
	    }
	}
	pos++;
    }
    return -1;
}


//*****************************************************************************
// int StringMatch::CompareWord(char *string, int &which, int &length)
//
int StringMatch::CompareWord(char *string, int &which, int &length)
{
    which = -1;
    length = -1;

    if (!table[0])
	return 0;

    int		state = 0;
    int		position = 0;

    //
    // Skip to at least the start of a word.
    //
    while ((unsigned char)string[position])
    {
	state = table[trans[(unsigned char)string[position]]][state];
	if (state == 0)
	{
	    return 0;
	}
	
	if (state & MATCH_INDEX_MASK)
	{
	    //
	    // Matched one of the patterns.  See if it is a word.
	    //
	    int	isWord = 1;

	    if ((unsigned char)string[position + 1])
	    {
		if (HtIsStrictWordChar((unsigned char)string[position + 1]))
		    isWord = 0;
	    }

	    if (isWord)
	    {
		which = ((unsigned int) (state & MATCH_INDEX_MASK)
			 >> INDEX_SHIFT) - 1;
		length = position + 1;
		return 1;
	    }
	    else
	    {
		//
		// Not at the end of a word.  Continue searching.
		//
		if ((state & STATE_MASK) != 0)
		{
		    state &= STATE_MASK;
		}
		else
		{
		    return 0;
		}
	    }
	}
	position++;
    }
    return 0;
}


//*****************************************************************************
// void StringMatch::TranslationTable(char *table)
//
void StringMatch::TranslationTable(char *table)
{
    if (local_alloc)
	delete [] trans;
    trans = (unsigned char *) table;
    local_alloc = 0;
}


//*****************************************************************************
// void StringMatch::IgnoreCase()
//   Set up the case translation table to convert uppercase to lowercase
//
void StringMatch::IgnoreCase()
{
    if (!local_alloc || !trans)
    {
	trans = new unsigned char[256];
	for (int i = 0; i < 256; i++)
	    trans[i] = (unsigned char)i;
	local_alloc = 1;
    }
    for (int i = 0; i < 256; i++)
	if (isupper((unsigned char)i))
	    trans[i] = tolower((unsigned char)i);
}


//*****************************************************************************
// void StringMatch::IgnorePunct(char *punct)
//   Set up the character translation table to ignore punctuation
//
void StringMatch::IgnorePunct(char *punct)
{
    if (!local_alloc || !trans)
    {
	trans = new unsigned char[256];
	for (int i = 0; i < 256; i++)
	    trans[i] = (unsigned char)i;
	local_alloc = 1;
    }
    if (punct)
	for (int i = 0; punct[i]; i++)
	    trans[(unsigned char)punct[i]] = 0;
    else
	for (int i = 0; i < 256; i++)
	    if (HtIsWordChar(i) && !HtIsStrictWordChar(i))
		trans[i] = 0;
}


//*****************************************************************************
// int StringMatch::FindFirst(char *source)
//
int StringMatch::FindFirst(char *source)
{
    int		dummy;
    return FindFirst(source, dummy, dummy);
}


//*****************************************************************************
// int StringMatch::Compare(char *source)
//
int StringMatch::Compare(char *source)
{
    int		dummy;
    return Compare(source, dummy, dummy);
}
