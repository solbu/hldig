//
// HTML.h
//
// $Id: HTML.h,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: HTML.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _HTML_h_
#define _HTML_h_

#include "Parsable.h"

class Retriever;
class URL;


class HTML : public Parsable
{
public:
    //
    // Construction/Destruction
    //
                        HTML();
    virtual		~HTML();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &baseURL);

private:
    //
    // Our state variables
    //
    String		word;
    URL			*href;
    String		title;
    String		description;
    String		head;
    String		tag;
    int			in_title;
    int			in_ref;
    int			in_heading;
    int			doindex;
    int			minimumWordLength;
    URL			*base;
    
    //
    // Helper functions
    //
    void		do_tag(Retriever &, String &);
};

#endif


