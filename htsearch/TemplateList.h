//
// TemplateList.h
//
// $Id: TemplateList.h,v 1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: TemplateList.h,v $
// Revision 1.1  1997/02/03 17:11:05  turtle
// Initial revision
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

    void	       	createFromString(char *);
    Template		*get(char *internalName);
	
    List	       	displayNames;
    List	       	internalNames;
    List	       	templates;
};

#endif


