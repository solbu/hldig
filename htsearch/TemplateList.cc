//
// TemplateList.cc
//
//
// TemplateList: As it sounds--a list of search result templates. Reads the 
//               configuration and any template files from disk, then retrieves
//               the relevant template for display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: TemplateList.cc,v 1.10 2003/06/24 19:58:07 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
TemplateList::get(const String& internalName)
{
    for (int i = 0; i < internalNames.Count(); i++)
    {
	const String	*s = (const String *) internalNames[i];
	if (mystrcasecmp(*s, internalName) == 0)
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
TemplateList::createFromString(const String& str)
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
		
	if (mystrcasecmp((char*)file, "builtin-long") == 0)
	{
	    String	s;
	    s << "<dl><dt><strong><a href=\"$&(URL)\">$&(TITLE)</a></strong>";
	    s << "$(STARSLEFT)\n";
	    s << "</dt><dd>$(EXCERPT)<br>\n";
	    s << "<em><a href=\"$&(URL)\">$&(URL)</a></em>\n";
	    s << " <font size=\"-1\">$(MODIFIED), $(SIZE) bytes</font>\n";
	    s << "</dd></dl>\n";
	    t->setMatchTemplate((char*)s);
	}
	else if (mystrcasecmp((char*)file, "builtin-short") == 0)
	{
	    t->setMatchTemplate("$(STARSRIGHT) <strong><a href=\"$&(URL)\">$&(TITLE)</a></strong><br>\n");
	}
	else
	{
	    t->createFromFile((char*)file);
	}
	templates.Add(t);
    }
    
    return 1;
}
