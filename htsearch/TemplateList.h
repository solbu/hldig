//
// TemplateList.h
//
// TemplateList: Holds the templates available to format a list of
//               results. These can be compiled in or read from
//               files.
//
// $Id: TemplateList.h,v 1.3 1999/09/09 10:16:07 loic Exp $
//
// $Log: TemplateList.h,v $
// Revision 1.3  1999/09/09 10:16:07  loic
// update comments
//
// Revision 1.2  1999/01/17 20:29:37  ghutchis
// Ensure template_map config has three members for each template we add,
// contributed by <tlm@mbox.comune.prato.it>.
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _TemplateList_h_
#define _TemplateList_h_

#include "Template.h"
#include <Object.h>
#include <List.h>

class TemplateList : public Object
{
public:
    TemplateList();
    ~TemplateList();

    int	       	        createFromString(char *);
    Template		*get(char *internalName);
	
    List	       	displayNames;
    List	       	internalNames;
    List	       	templates;
};

#endif


