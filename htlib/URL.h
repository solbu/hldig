//
// URL.h
//
// $Id: URL.h,v 1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: URL.h,v $
// Revision 1.1  1997/02/03 17:11:04  turtle
// Initial revision
//
// Revision 1.0  1995/08/22 17:08:01  turtle
// Support for HTTP only
//
//
#ifndef _URL_h_
#define _URL_h_

#include <String.h>


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
};


void encodeURL(String &, char *valid = "?_@.=&/:");
void decodeURL(String &);

#endif


