//
// Soundex.cc
//
// Implementation of Soundex
//
// $Log: Soundex.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Soundex.cc,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $";
#endif

#include "Soundex.h"
#include <Dictionary.h>


//*****************************************************************************
// Soundex::Soundex()
//
Soundex::Soundex()
{
    name = "soundex";
}


//*****************************************************************************
// Soundex::~Soundex()
//
Soundex::~Soundex()
{
}


//*****************************************************************************
// void Soundex::generateKey(char *word, String &key)
//
void
Soundex::generateKey(char *word, String &key)
{
    key = 0;
    if (word)
    {
	key << *word++;
    }
    else
    {
	key = "0";
	return;
    }

    while (key.length() < 6)
    {
	switch (*word)
	{
	    case 'b':
	    case 'p':
	    case 'f':
	    case 'v':
		key << '1';
		break;

	    case 'c':
	    case 's':
	    case 'k':
	    case 'g':
	    case 'j':
	    case 'q':
	    case 'x':
	    case 'z':
		key << '2';
		break;

	    case 'd':
	    case 't':
		key << '3';
		break;

	    case 'l':
		key << '4';
		break;

	    case 'm':
	    case 'n':
		key << '5';
		break;

	    case 'r':
		key << '6';
		break;

	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'y':
	    case 'w':
	    case 'h':
		break;
	}
	if (*word)
	    word++;
	else
	    break;
    }
}


//*****************************************************************************
// void Soundex::addWord(char *word)
//
void
Soundex::addWord(char *word)
{
    if (!dict)
    {
	dict = new Dictionary;
    }

    String	key;
    generateKey(word, key);

    String	*s = (String *) dict->Find(key);
    if (s)
    {
	if (mystrcasestr(s->get(), word) != 0)
	    (*s) << ' ' << word;
    }
    else
    {
	dict->Add(key, new String(word));
    }
}
