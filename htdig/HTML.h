//
// HTML.h
//
// Interface to the ht://Dig HTML parser
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HTML.h,v 1.6 1999/06/27 19:51:19 ghutchis Exp $
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
    String		meta_dsc;
    String		tag;
    int			in_title;
    int			in_ref;
    int			in_heading;
    int			doindex;
    int                 dofollow;
    unsigned int	minimumWordLength;
    URL			*base;
    
    //
    // Helper functions
    //
    void		do_tag(Retriever &, String &);
};

#endif


