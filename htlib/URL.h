//
// URL.h
//
// $Id: URL.h,v 1.4.2.2 2001/09/27 22:02:11 grdetil Exp $
//
// $Log: URL.h,v $
// Revision 1.4.2.2  2001/09/27 22:02:11  grdetil
// * htlib/Makefile.in, htlib/HtRegex.cc, htlib/HtRegex.h,
//   htlib/HtRegexReplace.cc, htlib/HtRegexReplace.h,
//   htlib/HtRegexReplaceList.cc, htlib/HtRegexReplaceList.h,
//   htlib/HtURLRewriter.cc, htlib/HtURLRewriter.h: Added new classes to
//   support regular expressions and implement url_rewrite_rules attribute,
//   using Geoff's variation of Andy Armstrong's implementation of this.
// * htlib/URL.h, htlib/URL.cc: Added URL::rewrite() method.
// * htlib/htString.h: Added Nth() method for HtRegex class.
// * htdig/Retriever.cc (got_href, got_redirect): Added calls to
//   url.rewrite(), and debugging output for this.
// * htdig/htdig.cc: Added calls to make instance of HtURLRewriter class.
// * htdoc/attrs.html, htdoc/cf_byname.html, htdoc/cf_byprog.html,
//   htcommon/defaults.cc: Added url_rewrite_rules attribute.
//
// Revision 1.4.2.1  2000/02/16 21:14:59  grdetil
// htlib/URL.h (encodeURL): Change list of valid characters to include only
// 	unreserved ones.
// htlib/cgi.cc (init): Allow "&" and ";" as input parameter separators.
// htsearch/Display.cc (createURL): Encode each parameter separately,
// 	using new unreserved list, before piecing together query string, to
// 	allow characters like "?=&" within parameters to be encoded.
//
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
    void		rewrite();
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


// Unreserved punctuation allowed unencoded in URLs.  We use a more restricted
// list of unreserved characters than allowed by RFC 2396 (which revises and
// replaces RFC 1738), because it can't hurt to encode any of these
// characters, and they can pose problems in some contexts.  RFC 2396 says
// that only alphanumerics, the unreserved characters "-_.!~*'(),", and
// reserved characters used for their reserved purposes may be used
// unencoded within a URL.  We encode reserved characters because we now
// encode URL parameter values individually before piecing together the whole
// query string using reserved characters.

#define UNRESERVED	"-_.!~*"

//void encodeURL(String &, char *valid = "?_@.=&/:");
void encodeURL(String &, char *valid = UNRESERVED);
void decodeURL(String &);

#endif


