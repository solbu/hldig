//
// ParsedString.h
//
// ParsedString: Contains a string. The string my contain $var, ${var}, $(var)
//               `filename`. The get method will expand those using the
//               dictionary given in argument.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ParsedString.h,v 1.4 1999/09/11 05:03:52 ghutchis Exp $

#ifndef _ParsedString_h_
#define _ParsedString_h_

#include "Object.h"
#include "htString.h"
#include "Dictionary.h"

class ParsedString : public Object
{
public:
	//
	// Construction/Destruction
	//
					ParsedString();
					ParsedString(char *s);
					~ParsedString();

	void			set(char *s);
	char			*get(Dictionary &d);
protected:
	String			value;
	String			parsed;

private:
	void			getFileContents(String &, char *);
};

#endif


