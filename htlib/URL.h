//
// URL.h
//
// URL: A URL parsing class, implementing as closely as possible the standard
//      laid out in RFC2396 (e.g. http://www.faqs.org/rfcs/rfc2396.html)
//      including support for multiple schemes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: URL.h,v 1.8 1999/09/24 10:29:03 loic Exp $
//

#ifndef _URL_h_
#define _URL_h_

#include "htString.h"

class URL
{
public:
    URL();
    URL(const char *url);
    URL(URL &url);
    URL(const char *ref, URL &parent);

    void		parse(const char *url);

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
    char		*user()			{return _user;}
    void		user(char *u) 		{_user = u;}

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
    String		_user;

    void		removeIndex(String &);
    void                normalizePath();
    void		ServerAlias();
    void		constructURL();
};


void encodeURL(String &, char *reserved = ";/?:@&=+$,");
	       //	       char *unreserved = "-_.!~*'()");
void decodeURL(String &);

#endif


