//
// Postscript.h
//
// $Id: Postscript.h,v 1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: Postscript.h,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#ifndef _Postscript_h_
#define _Postscript_h_

#include "Parsable.h"

class StringList;
class URL;

class Postscript : public Parsable
{
public:
    //
    // Construction/Destruction
    //
    Postscript();
    ~Postscript();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &);
	
private:
    void 		parse_line(Retriever &, String &line);
    void		tokenize(String &, StringList &);
    void		flush_word(Retriever &);
    void		parse_string(Retriever &, String &str);

    String		word;
    String		head;
    int			offset;
    int			in_space;
    int			generatorType;
    String		last_t;
    String		last_y;
    char		*valid_punctuation;
};

#endif


