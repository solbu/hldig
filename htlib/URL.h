//
// URL.h
//
// $Id: URL.h,v 1.4 1998/10/21 16:34:19 bergolth Exp $
//
// $Log: URL.h,v $
// Revision 1.4  1998/10/21 16:34:19  bergolth
// Added translation of server names. Additional limiting after normalization of the URL.
//
// Revision 1.3  1997/12/11 00:28:59  turtle
// Added double slash removal code.  These were causing loops.
//
// Revision 1.2  1997/03/24 04:33:22  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
// Revision 1.0  1995/08/22 17:08:01  turtle
// Support for HTTP only
//
//
#ifndef _URL_h_
#define _URL_h_

#include "htString.h"


class URL
{
public:
    URL();
    URL(char *url);
    URL(URL &url);
    URL(char *ref, URL &parent);

    void		parse(char *url);

    char		*host()			{return _host;}
    void		host(char *h)		{_host = h;}
    int			port()			{return _port;}
    void		port(int p)		{_port = p;}
    char		*service()		{return _service;}
    void		service(char *s)	{_service = s;}
    char		*path()			{return _path;}
    void		path(char *p);
    int			hopcount()		{return _hopcount;}
    void		hopcount(int h)		{_hopcount = h;}

    char		*get()			{return _url;}
    void		dump();
    void		normalize();
    char		*signature();

private:
    String		_url;
    String		_path;
    String		_service;
    String		_host;
    int			_port;
    int			_normal;
    int			_hopcount;
    String		_signature;

    void		removeIndex(String &);
    void                normalizePath();
    void		ServerAlias();
};


void encodeURL(String &, char *valid = "?_@.=&/:");
void decodeURL(String &);

#endif


