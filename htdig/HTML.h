//
// HTML.h
//
// HTML: Class to parse HTML documents and return useful information 
//       to the Retriever
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HTML.h,v 1.11 2003/01/20 22:40:14 lha Exp $
//
#ifndef _HTML_h_
#define _HTML_h_

#include "Parsable.h"
#include "QuotedStringList.h"

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
    int			noindex;
    int                 nofollow;
    unsigned int	minimumWordLength;
    URL			*base;
    QuotedStringList	skip_start;
    QuotedStringList	skip_end;

    //
    // Helper functions
    //
    void		do_tag(Retriever &, String &);
    const String	transSGML(const String& str);
};

#endif


