//
// ParsedString.h
//
// ParsedString: Contains a string. The string my contain $var, ${var}, $(var)
//               `filename`. The get method will expand those using the
//               dictionary given in argument.
// 
// $Id: ParsedString.h,v 1.3 1999/09/08 14:42:29 loic Exp $
//
// $Log: ParsedString.h,v $
// Revision 1.3  1999/09/08 14:42:29  loic
// update comments
//
// Revision 1.2  1997/03/24 04:33:21  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
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


