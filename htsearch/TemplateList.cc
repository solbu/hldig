//
// TemplateList.cc
//
// Implementation of TemplateList
//
// $Log: TemplateList.cc,v $
// Revision 1.4  1999/01/17 20:29:37  ghutchis
// Ensure template_map config has three members for each template we add,
// contributed by <tlm@mbox.comune.prato.it>.
//
// Revision 1.3  1998/09/10 04:16:26  ghutchis
//
// More bug fixes.
//
// Revision 1.1  1997/02/03 17:11:05  turtle
// Initial revision
//
//
#if RELEASE
static char RCSid[] = "$Id: TemplateList.cc,v 1.4 1999/01/17 20:29:37 ghutchis Exp $";
#endif

#include "TemplateList.h"
#include <URL.h>
#include <StringList.h>

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
    StringList	sl(str, "\t \r\n");
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
		
	if (mystrcasecmp(internal, "builtin-long") == 0)
	{
	    String	s;
	    s << "<dl><dt><strong><a href=\"$(URL)\">$(TITLE)</a></strong>";
	    s << "$(STARSLEFT)\n";
	    s << "</dt><dd>$(EXCERPT)<br>\n";
	    s << "<i><a href=\"$(URL)\">$(URL)</a></i>\n";
	    s << " <font size=-1>$(MODIFIED), $(SIZE) bytes</font>\n";
	    s << "</dd></dl>\n";
	    t->setMatchTemplate(s);
	}
	else if (mystrcasecmp(internal, "builtin-short") == 0)
	{
	    t->setMatchTemplate("$(STARSRIGHT) <strong><a href=\"$(URL)\">$(TITLE)</a></strong><br>\n");
	}
	else
	{
	    t->createFromFile(file);
	}
	templates.Add(t);
    }
    
    return 1;
}
