//
// ParsedString.h
//
// $Id: ParsedString.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: ParsedString.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _ParsedString_h_
#define _ParsedString_h_

#include "Object.h"
#include "String.h"
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


