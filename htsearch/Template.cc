//
// Template.cc
//
// Template: A template to set the display of the search results.
//           MatchTemplate is used for every match, Start and End templates
//           are used between the header and the first match and the 
//           last match and the footer respectively.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Template.cc,v 1.7 2003/06/24 19:58:07 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
Template::createFromFile(const char *filename)
{
    String	realFile;

    realFile = filename;
    realFile << ".start";
    readFile(startTemplate, (char*)realFile);

    realFile = filename;
    realFile << ".end";
    readFile(endTemplate, (char*)realFile);

    readFile(matchTemplate, filename);
}

//*****************************************************************************
// Append the contents of a file to a string.  Nothing happens if the file
// doesn't exist.
//
void
Template::readFile(String &s, const char *filename) const
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



