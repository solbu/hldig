//
// Template.h
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
// $Id: Template.h,v 1.7 2003/06/24 19:58:07 nealr Exp $
//

#ifndef _Template_h_
#define _Template_h_

#include "Object.h"
#include "htString.h"

//
// This class holds information about output templates.
//
class Template : public Object
{
public:
    Template();
    ~Template();

    const String&      	getMatchTemplate() const       	{ return matchTemplate; }
    const String&     	getStartTemplate() const       	{ return startTemplate; }
    const String&      	getEndTemplate() const       	{ return endTemplate; }

    void	       	setMatchTemplate(const char *s)	{ matchTemplate = s; }
    void	       	setStartTemplate(const char *s)	{ startTemplate = s; }
    void	       	setEndTemplate(const char *s)	{ endTemplate = s; }

    void	       	createFromFile(const char *filename);
	
protected:
    String	       	matchTemplate;
    String	       	startTemplate;
    String	       	endTemplate;

private:
    void	       	readFile(String &, const char *) const;
};

#endif


