//
// io.h
//
// $Id: io.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: io.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _io_h_
#define _io_h_

#include "Object.h"

class String;

class io : public Object
{
public:
    //
    // Construction/Destruction
    //
    io();
    ~io();

    //
    // Routines all derived class will have in common
    //
    String		*read_line(String &, char *terminator = "\n");
    char			*read_line(char *buffer, int maxlength, char *terminator = "\n");
    String		*read_line(char *terminator = "\n");
    virtual int		read_char();
    int			write_line(char *buffer, char *eol = "\n");

    int			write(char *buffer, int maxlength = -1);
    int			read(char *buffer, int maxlength);

    //
    // Routines which all derived classes should override
    //
    virtual int		read_partial(char *buffer, int maxlength) = 0;
    virtual int		write_partial(char *buffer, int maxlength) = 0;
    virtual int		close() = 0;

private:
    //
    // For buffered IO we will need a buffer
    //
    enum {BUFFER_SIZE = 8192};
    char			buffer[BUFFER_SIZE];
    int			pos, pos_max;
};

#endif
