//
// TemplateList.cc
//
// TemplateList: As it sounds--a list of search result templates. Reads the 
//               configuration and any template files from disk, then retrieves
//               the relevant template for display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
#if RELEASE
static char RCSid[] = "$Id: TemplateList.cc,v 1.4.2.5 2001/11/21 17:40:45 grdetil Exp $";
#endif

#include "TemplateList.h"
#include "URL.h"
#include "QuotedStringList.h"

//*****************************************************************************
TemplateList::TemplateList()
{
}


//*****************************************************************************
TemplateList::~TemplateList()
{
}


//*****************************************************************************
// Return the template that belongs to the given internal template
// name.  If no template can be found, NULL is returned.
//
Template *
TemplateList::get(char *internalName)
{
    for (int i = 0; i < internalNames.Count(); i++)
    {
	String	*s = (String *) internalNames[i];
	if (mystrcasecmp(s->get(), internalName) == 0)
	    return (Template *) templates[i];
    }
    return 0;
}


//*****************************************************************************
// Create a list of templates from a configuration string.  The string
// will have triplets of: display name, internal name, and filename.
// There are two special cases for the internal name: builtin-long and
// builtin-short.  These will cause a hardcoded template to be
// created.  All other templates are read in from the specified
// filename.
//
int
TemplateList::createFromString(char *str)
{
    QuotedStringList	sl(str, "\t \r\n");
    String		display, internal, file;
    Template	*t;

    if ( sl.Count() % 3) return 0; // Make sure we have a multiple of three

    for (int i = 0; i < sl.Count(); i += 3)
    {
	display = sl[i];
	decodeURL(display);
	internal = sl[i + 1];
	file = sl[i + 2];
	displayNames.Add(new String(display));
	internalNames.Add(new String(internal));

	t = new Template();
		
	if (mystrcasecmp(file, "builtin-long") == 0)
	{
	    String	s;
	    s << "<dl><dt><strong><a href=\"$&(URL)\">$&(TITLE)</a></strong>";
	    s << "$(STARSLEFT)\n";
	    s << "</dt><dd>$(EXCERPT)<br>\n";
	    s << "<em><a href=\"$&(URL)\">$&(URL)</a></em>\n";
	    s << " <font size=\"-1\">$(MODIFIED), $(SIZE) bytes</font>\n";
	    s << "</dd></dl>\n";
	    t->setMatchTemplate(s);
	}
	else if (mystrcasecmp(file, "builtin-short") == 0)
	{
	    t->setMatchTemplate("$(STARSRIGHT) <strong><a href=\"$&(URL)\">$&(TITLE)</a></strong><br>\n");
	}
	else
	{
	    t->createFromFile(file);
	}
	templates.Add(t);
    }
    
    return 1;
}
