//
// TemplateList.h
//
// TemplateList: As it sounds--a list of search result templates. Reads the 
//               configuration and any template files from disk, then retrieves
//               the relevant template for display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: TemplateList.h,v 1.4 1999/09/10 17:22:25 ghutchis Exp $
//

#ifndef _TemplateList_h_
#define _TemplateList_h_

#include "Template.h"
#include "Object.h"
#include "List.h"

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


