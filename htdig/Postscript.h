//
// Postscript.h
//
// $Id: Postscript.h,v 1.1.1.1.2.1 1999/03/23 23:22:54 grdetil Exp $
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
};

#endif


