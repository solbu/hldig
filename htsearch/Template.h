//
// Template.h
//
// Template: A template to set the display of the search results.
//           MatchTemplate is used for every match, Start and End templates
//           are used between the header and the first match and the 
//           last match and the footer respectively.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Template.h,v 1.4 1999/09/10 17:22:25 ghutchis Exp $
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

    char	       	*getMatchTemplate()	       	{return matchTemplate;}
    char	       	*getStartTemplate()	       	{return startTemplate;}
    char	       	*getEndTemplate()	       	{return endTemplate;}

    void	       	setMatchTemplate(char *s)	{matchTemplate = s;}
    void	       	setStartTemplate(char *s)	{startTemplate = s;}
    void	       	setEndTemplate(char *s)		{endTemplate = s;}

    void	       	createFromFile(char *filename);
	
protected:
    String	       	matchTemplate;
    String	       	startTemplate;
    String	       	endTemplate;

private:
    void	       	readFile(String &, char *);
};

#endif


