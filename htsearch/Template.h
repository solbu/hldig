//
// Template.h
//
// $Id: Template.h,v 1.2 1997/03/24 04:33:24 turtle Exp $
//
// $Log: Template.h,v $
// Revision 1.2  1997/03/24 04:33:24  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _Template_h_
#define _Template_h_

#include <Object.h>
#include <htString.h>

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


