//
// io.cc
//
// Implementation of io
//
// $Log: io.cc,v $
// Revision 1.2  1997/03/24 04:33:23  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: io.cc,v 1.2 1997/03/24 04:33:23 turtle Exp $";
#endif

#include "io.h"
#include "htString.h"
#include <errno.h>

#undef MIN
#define	MIN(a,b)		((a)<(b)?(a):(b))

//*******************************************************************************
// io::io()
//
io::io()
{
    pos = pos_max = 0;
}


//*******************************************************************************
// io::~io()
//
io::~io()
{
}


//*****************************************************************************
// int io::read_char()
//
int io::read_char()
{
    if (pos >= pos_max)
    {
	pos_max = read_partial(buffer, sizeof(buffer));
	pos = 0;
	if (pos_max <= 0)
	{
	    return -1;
	}
    }
    return buffer[pos++] & 0xff;
}


//*****************************************************************************
// String *io::read_line(String &s, char *terminator)
//
String *io::read_line(String &s, char *terminator)
{
    int		termseq = 0;
    s = 0;

    for (;;)
    {
	int	ch = read_char();
	if (ch < 0)
	{
	    //
	    // End of file reached.  If we still have stuff in the input buffer
	    // we need to return it first.  When we get called again we will
	    // return NULL to let the caller know about the EOF condition.
	    //
	    if (s.length())
		break;
	    else
		return (String *) 0;
	}
	else if (terminator[termseq] && ch == terminator[termseq])
	{
	    //
	    // Got one of the terminator characters.  We will not put
	    // it in the string but keep track of the fact that we
	    // have seen it.
	    //
	    termseq++;
	    if (!terminator[termseq])
		break;
	}
	else
	{
	    s << (char) ch;
	}
    }

    return &s;
}


//*****************************************************************************
// String *io::read_line(char *terminator)
//
String *io::read_line(char *terminator)
{
    String	*s;

    s = new String;
    return read_line(*s, terminator);
}


//*****************************************************************************
// char *io::read_line(char *buffer, int maxlength, char *terminator)
//
char *io::read_line(char *buffer, int maxlength, char *terminator)
{
    char	*start = buffer;
    int		termseq = 0;

    while (maxlength > 0)
    {
	int		ch = read_char();
	if (ch < 0)
	{
	    //
	    // End of file reached.  If we still have stuff in the input buffer
	    // we need to return it first.  When we get called again, we will
	    // return NULL to let the caller know about the EOF condition.
	    //
	    if (buffer > start)
		break;
	    else
		return (char *) 0;
	}
	else if (terminator[termseq] && ch == terminator[termseq])
	{
	    //
	    // Got one of the terminator characters.  We will not put
	    // it in the string but keep track of the fact that we
	    // have seen it.
	    //
	    termseq++;
	    if (!terminator[termseq])
		break;
	}
	else
	{
	    *buffer++ = ch;
	    maxlength--;
	}
    }
    *buffer = '\0';

    return start;
}


//*****************************************************************************
// int io::write_line(char *str, char *eol)
//
int io::write_line(char *str, char *eol)
{
    int		n, nn;

    if ((n = write(str)) < 0)
	return -1;

    if ((nn = write(eol)) < 0)
	return -1;

    return n + nn;
}


//*****************************************************************************
// int io::write(char *buffer, int length)
//
int io::write(char *buffer, int length)
{
    int		nleft, nwritten;

    if (length == -1)
	length = strlen(buffer);

    nleft = length;
    while (nleft > 0)
    {
	nwritten = write_partial(buffer, nleft);
	if (nwritten < 0 && errno == EINTR)
	    continue;
	if (nwritten <= 0)
	    return nwritten;
	nleft -= nwritten;
	buffer += nwritten;
    }
    return length - nleft;
}


//*****************************************************************************
// int io::read(char *buffer, int length)
//
int io::read(char *buffer, int length)
{
    int		nleft, nread;

    nleft = length;

    //
    // If there is data in our internal input buffer, use that first.
    //
    if (pos < pos_max)
    {
	int n = MIN(length, pos_max - pos);

	memcpy(buffer, &this->buffer[pos], n);
	pos += n;
	buffer += n;
	nleft -= n;
    }

    while (nleft > 0)
    {
	nread = read_partial(buffer, nleft);
	if (nread < 0 && errno == EINTR)
	    continue;
	if (nread < 0)
	    return -1;
	else if (nread == 0)
	    break;

	nleft -= nread;
	buffer += nread;
    }
    return length - nleft;
}


