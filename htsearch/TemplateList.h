//
// TemplateList.h
//
// TemplateList: As it sounds--a list of search result templates. Reads the 
//               configuration and any template files from disk, then retrieves
//               the relevant template for display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: TemplateList.h,v 1.5.2.1 2000/10/20 03:40:59 ghutchis Exp $
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

    int	       	        createFromString(const String& str);
    Template		*get(const String& internalName);
	
    List	       	displayNames;
    List	       	internalNames;
    List	       	templates;
};

#endif


