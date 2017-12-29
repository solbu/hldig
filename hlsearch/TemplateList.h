//
// TemplateList.h
//
// TemplateList: As it sounds--a list of search result templates. Reads the 
//               configuration and any template files from disk, then retrieves
//               the relevant template for display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: TemplateList.h,v 1.8 2004/05/28 13:15:24 lha Exp $
//

#ifndef _TemplateList_h_
#define _TemplateList_h_

#include "Template.h"
#include "Object.h"
#include "List.h"

class TemplateList:public Object
{
public:
  TemplateList ();
  ~TemplateList ();

  int createFromString (const String & str);
  Template *get (const String & internalName);

  List displayNames;
  List internalNames;
  List templates;
};

#endif
