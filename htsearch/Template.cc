//
// Template.cc
//
// Implementation of Template
//
// $Log: Template.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Template.cc,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $";
#endif

#include "Template.h"
#include <stdio.h>


//*****************************************************************************
Template::Template()
{
}


//*****************************************************************************
Template::~Template()
{
}


//*****************************************************************************
// The start and end templates are created from the filename of the
// main template by appending ".start" and ".end" to the filename
// respectively.
//
void
Template::createFromFile(char *filename)
{
    String	realFile;

    realFile = filename;
    realFile << ".start";
    readFile(startTemplate, realFile);

    realFile = filename;
    realFile << ".end";
    readFile(endTemplate, realFile);

    readFile(matchTemplate, filename);
}

//*****************************************************************************
// Append the contents of a file to a string.  Nothing happens if the file
// doesn't exist.
//
void
Template::readFile(String &s, char *filename)
{
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];

    if (!fl)
	return;
    s = 0;
    while (fgets(buffer, sizeof(buffer), fl))
    {
	s << buffer;
    }
    fclose(fl);
}



