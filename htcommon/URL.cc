//
// URL.cc
//
// URL: A URL parsing class, implementing as closely as possible the standard
//      laid out in RFC2396 (e.g. http://www.faqs.org/rfcs/rfc2396.html)
//      including support for multiple services. (schemes in the RFC)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: URL.cc,v 1.3.2.12 2000/09/01 21:32:31 angus Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "URL.h"
#include "Dictionary.h"
#include "HtConfiguration.h"
#include "StringMatch.h"
#include "StringList.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

extern HtConfiguration	config;

#define NNTP_DEFAULT_PORT 119

//*****************************************************************************
// URL::URL()
// Default Constructor
//
URL::URL()
{
  _port = 0;
  _normal = 0;
  _hopcount = 0;
}


//*****************************************************************************
// URL::URL(URL &nurl)
// Copy constructor
//
URL::URL(URL &nurl)
{
    _service = nurl._service;
    _user = nurl._user;
    _host = nurl._host;
    _port = nurl._port;
    _url = nurl._url;
    _path = nurl._path;
    _normal = nurl._normal;
    _signature = nurl._signature;
    _hopcount = nurl._hopcount;
}


//*****************************************************************************
// URL::URL(const char *nurl)
// Construct a URL from a String (obviously parses the string passed in)
// 
URL::URL(const char *nurl)
{
    _port = 0;
    _normal = 0;
    _hopcount = 0;
    parse(nurl);
}


//*****************************************************************************
// URL::URL(char *ref, URL &parent)
//   Parse a reference given a parent url.  This is needed to resolve relative
//   references which do NOT have a full url.
//
URL::URL(const char *url, URL &parent)
{
    String	temp(url);
    temp.remove(" \r\n\t");
    char* ref = temp;

    // Grab as much from the original URL as possible
    _service = parent._service;
    _user = parent._user;
    _host = parent._host;
    _port = parent._port;
    _normal = parent._normal;
    _signature = parent._signature;
    // Since this is one hop *after* the parent, we should account for this
    _hopcount = parent._hopcount + 1;

    //
    // Strip any optional anchor from the reference.  If, however, the
    // reference contains CGI parameters after the anchor, the parameters
    // will be moved left to replace the anchor.  The overall effect is that
    // the anchor is removed.
    // Thanks goes to David Filiatrault <dwf@WebThreads.Com> for suggesting
    // this removal process.
    //
    char	*anchor = strchr(ref, '#');
    char	*params = strchr(ref, '?');
    if (anchor)
    {
	*anchor = '\0';
	if (params)
	{
	    if (anchor < params)
	    {
		while (*params)
		{
		    *anchor++ = *params++;
		}
		*anchor = '\0';
	    }
	}
    }

    //
    // If, after the removal of a possible '#' we have nothing left,
    // we just want to use the base URL (we're on the same page but
    // different anchors)
    //
    if (!*ref)
    {
        // We've already copied much of the info
	_url = parent._url;
	_path = parent._path;
	// Since this is on the same page, we want the same hopcount
	_hopcount = parent._hopcount;
	return;
    }

    // OK, now we need to work out what type of child URL this is
    char	*p = ref;
    while (isalpha(*p))  // Skip through the service portion
	p++;
    int	hasService = (*p == ':');
    if (hasService && ((strncmp(ref, "http://", 7) == 0) ||
		       (strncmp(ref, "http:", 5) != 0)))
    {
	//
	// No need to look at the parent url since this is a complete url...
	//
	parse(ref);
    }
    else if (strncmp(ref, "//", 2) == 0)
    {
	// look at the parent url's _service, to make this is a complete url...
	String	fullref(parent._service);
	fullref << ':' << ref;
	parse((char*)fullref);
    }
    else
    {
	if (hasService)
	    ref = p + 1;	// Relative URL, skip "http:"
	
	//
	// Remove any leading "./" sequences which could get us into
	// recursive loops.
	//
	while (strncmp(ref, "./", 2) == 0)
	    ref += 2;

	if (*ref == '/')
	{
	    //
	    // The reference is on the same server as the parent, but
	    // an absolute path was given...
	    //
	    _path = ref;
	}
	else
	{
	    //
	    // The reference is relative to the parent
	    //
	    _path = parent._path;
	    int i = _path.indexOf('?');
	    if (i >= 0)
	    {
		_path.chop(_path.length() - i);
	    }
	    if (_path.last() == '/')
	    {
		//
		// Parent was a directory.  Easy enough: just append
		// the current ref to it
		//
		_path << ref;
	    }
	    else
	    {
		//
		// Parent was a file.  We need to strip the last part
		// of the path before we add the reference to it.
		//
		String	temp = _path;
		p = strrchr((char*)temp, '/');
		if (p)
		{
		    p[1] = '\0';
		    _path = temp.get();
		    _path << ref;
		}
		else
		{
		    //
		    // Something must be wrong since there were no '/'
		    // found in the parent url.
		    //
		    // We do nothing here.  The new url is the parent.
		    //
		}
	    }

            //
            // Get rid of loop-causing constructs in the path
            //
            normalizePath();
	}

	//
	// Build the url.  (Note, the host name has NOT been normalized!)
	// No need for this if we have called URL::parse.
	//
	constructURL();
    }
}


//*****************************************************************************
// void URL::parse(const char *u)
//   Given a URL string, extract the service, host, port, and path from it.
//
void URL::parse(const char *u)
{
    String	temp(u);
    temp.remove(" \t\r\n");
    char	*nurl = temp;

    //
    // Ignore any part of the URL that follows the '#' since this is just
    // an index into a document.
    //
    char	*p = strchr(nurl, '#');
    if (p)
	*p = '\0';

    // Some members need to be reset.  If not, the caller would
    // have used URL::URL(char *ref, URL &parent)
    // (which may call us, if the URL is found to be absolute).
    _normal = 0;
    _signature = 0;
    _user = 0;

    //
    // Extract the service
    //
    p = strchr(nurl, ':');
    if (p)
    {
	_service = strtok(nurl, ":");
	p = strtok(0, "\n");
    }
    else
    {
	_service = "http";
	p = strtok(nurl, "\n");
    }
    _service.lowercase();

    //
    // Extract the host
    //
    if (!p || strncmp(p, "//", 2) != 0)
    {
	// No host specified, it's all a path.
	_host = 0;
	_port = 0;
	_url = 0;
	_path = p;
    }
    else
    {
	p += 2;

	//
	// p now points to the host
	//
	char	*q = strchr(p, ':');
	char	*slash = strchr(p, '/');
    
	_path = "/";
	if (strcmp((char*)_service, "file") == 0)
	  {
	    // These should be of the form file:/// (i.e. no host)
	    // if there is a file://host/path then strip the host
	    if (strncmp(p, "/", 1) != 0)
	      {
		p = strtok(p, "/");
		_path << strtok(0, "\n");
	      }
	    else
	      _path << strtok(p, "\n");	      
	    _host = "localhost";
	    _port = 0;
	  }
	else if (q && ((slash && slash > q) || !slash))
	{
	    _host = strtok(p, ":");
	    p = strtok(0, "/");
	    if (p)
	      _port = atoi(p);
	    if (!p || _port <= 0)
	      _port = DefaultPort();

	    //
	    // The rest of the input string is the path.
	    //
	    _path << strtok(0, "\n");

	}
	else
	{
	    _host = strtok(p, "/");
	    _host.chop(" \t");
	    _port = DefaultPort();

	    //
	    // The rest of the input string is the path.
	    //
	    _path << strtok(0, "\n");

	}

	// Check to see if host contains a user@ portion
	int atMark = _host.indexOf('@');
	if (atMark != -1)
	  {
	    _user = _host.sub(0, atMark);
	    _host = _host.sub(atMark + 1);
	  }
    }

    //
    // Get rid of loop-causing constructs in the path
    //
    normalizePath();

    //
    // Build the url.  (Note, the host name has NOT been normalized!)
    //
    constructURL();
}


//*****************************************************************************
// void URL::normalizePath()
//
void URL::normalizePath()
{
    //
    // We now need to take care of situations where the URL contains
    // relative parts ("/../")
    // We will rewrite the path to be the minimal.
    //
    int	i, limit;
    int	leadingdotdot = 0;
    String	newPath;
    int	pathend = _path.indexOf('?');	// Don't mess up query strings.
    if (pathend < 0)
        pathend = _path.length();
    while ((i = _path.indexOf("/../")) >= 0 && i < pathend)
    {
        if ((limit = _path.lastIndexOf('/', i - 1)) >= 0)
        {
            newPath = _path.sub(0, limit).get();
            newPath << _path.sub(i + 3).get();
            _path = newPath;
        }
        else
        {
            _path = _path.sub(i + 3).get();
            leadingdotdot++;
        }
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }
    if ((i = _path.indexOf("/..")) >= 0 && i == pathend-3)
    {
        if ((limit = _path.lastIndexOf('/', i - 1)) >= 0)
            newPath = _path.sub(0, limit+1).get();	// keep trailing slash
        else
        {
            newPath = '/';
            leadingdotdot++;
        }
        newPath << _path.sub(i + 3).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }
    // The RFC gives us a choice of what to do when we have .. left and
    // we're at the top level. By principle of least surprise, we'll just
    // toss any "leftovers" Otherwise, we'd have a loop here to add them.

    //
    // Also get rid of redundant "/./".  This could cause infinite
    // loops.
    //
    while ((i = _path.indexOf("/./")) >= 0 && i < pathend)
    {
        newPath = _path.sub(0, i).get();
        newPath << _path.sub(i + 2).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }
    if ((i = _path.indexOf("/.")) >= 0 && i == pathend-2)
    {
        newPath = _path.sub(0, i+1).get();		// keep trailing slash
        newPath << _path.sub(i + 2).get();
        _path = newPath;
        pathend--;
    }

    //
    // Furthermore, get rid of "//".  This could also cause loops
    //
    while ((i = _path.indexOf("//")) >= 0 && i < pathend)
    {
        newPath = _path.sub(0, i).get();
        newPath << _path.sub(i + 1).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }

    // Finally change all "%7E" to "~" for sanity
    while ((i = _path.indexOf("%7E")) >= 0 && i < pathend)
      {
        newPath = _path.sub(0, i).get();
	newPath << "~";
        newPath << _path.sub(i + 3).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
      }

    // If the server *isn't* case sensitive, we want to lowercase the path
    if (!config.Boolean("case_sensitive", 1))
      _path.lowercase();

    // And don't forget to remove index.html or similar file.
    if (strcmp((char*)_service, "file") != 0)
	removeIndex(_path);
}

//*****************************************************************************
// void URL::dump()
//
void URL::dump()
{
    cout << "service = " << _service.get() << endl;
    cout << "user = " << _user.get() << endl;
    cout << "host = " << _host.get() << endl;
    cout << "port = " << _port << endl;
    cout << "path = " << _path << endl;
    cout << "url = " << _url << endl;
}


//*****************************************************************************
// void URL::path(char *newpath)
//
void URL::path(char *newpath)
{
    _path = newpath;
    if (!config.Boolean("case_sensitive",1))
      _path.lowercase();
    constructURL();
}


//*****************************************************************************
// void URL::removeIndex(String &path)
//   Attempt to remove the remove_default_doc from the end of a URL path.
//   This needs to be done to normalize the paths and make .../ the
//   same as .../index.html
//
void URL::removeIndex(String &path)
{
    static StringMatch *defaultdoc = 0;

    if (path.length() == 0 || strchr((char*)path, '?'))
	return;

    int filename = path.lastIndexOf('/') + 1;
    if (filename == 0)
        return;

    if (! defaultdoc)
    {
      StringList  l(config["remove_default_doc"], " \t");
      defaultdoc = new StringMatch();
      defaultdoc->IgnoreCase();
      defaultdoc->Pattern(l.Join('|'));
    }
    if (defaultdoc->hasPattern() &&
            defaultdoc->CompareWord((char*)path.sub(filename)))
	path.chop(path.length() - filename);
}


//*****************************************************************************
// void URL::normalize()
//   Make sure that URLs are always in the same format.
//
void URL::normalize()
{
    static int	hits = 0, misses = 0;

    if (_service.length() == 0 || _normal)
	return;

    if (strcmp((char*)_service, "http") != 0)
	return;

    removeIndex(_path);

    //
    // Convert a hostname to an IP address
    //
    _host.lowercase();

    if (!config.Boolean("allow_virtual_hosts", 1))
    {
	static Dictionary	hostbyname;
	unsigned long		addr;
	struct hostent		*hp;

	String	*ip = (String *) hostbyname[_host];
	if (ip)
	{
	    memcpy((char *) &addr, ip->get(), ip->length());
	    hits++;
	}
	else
	{
	    addr = inet_addr(_host.get());
	    if (addr == 0xffffffff)
	    {
		hp = gethostbyname(_host.get());
		if (hp == NULL)
		{
		    return;
		}
		memcpy((char *)&addr, (char *)hp->h_addr, hp->h_length);
		ip = new String((char *) &addr, hp->h_length);
		hostbyname.Add(_host, ip);
		misses++;
	    }
	}

	static Dictionary	machines;
	String			key;
	key << int(addr);
	String			*realname = (String *) machines[key];
	if (realname)
	    _host = realname->get();
	else
	    machines.Add(key, new String(_host));
    }
    ServerAlias();
    
    //
    // Reconstruct the url
    //
    constructURL();
    _normal = 1;
    _signature = 0;
}


//*****************************************************************************
// char *URL::signature()
//   Return a string which uniquely identifies the server the current
//   URL is refering to.
//   This is the first portion of a url: service://user@host:port/
//   (in short this is the URL pointing to the root of this server)
//
char *URL::signature()
{
    if (_signature.length())
	return _signature;

    if (!_normal)
	normalize();
    _signature = _service;
    _signature << "://";
    if (_user.length())
      _signature << _user << '@';
    _signature << _host;
    _signature << ':' << _port << '/';
    return _signature;
}

//*****************************************************************************
// void URL::ServerAlias()
// Takes care of the server aliases, which attempt to simplify virtual
// host problems
//
void URL::ServerAlias()
{
  static Dictionary *serveraliases= 0;

  if (! serveraliases)
    {
      String l= config["server_aliases"];
      String from, *to;
      serveraliases = new Dictionary();
      char *p = strtok(l, " \t");
      char *salias= NULL;
      while (p)
	{
	  salias = strchr(p, '=');
	  if (! salias)
	    {
	      p = strtok(0, " \t");
	      continue;
	    }
	  *salias++= '\0';
	  from = p;
	  if (from.indexOf(':') == -1)
	    from.append(":80");
	  to= new String(salias);
	  if (to->indexOf(':') == -1)
	    to->append(":80");
	  serveraliases->Add(from.get(), to);
	  // fprintf (stderr, "Alias: %s->%s\n", from.get(), to->get());
	  p = strtok(0, " \t");
	}
    }

  String *al= 0;
  int newport;
  int delim;
  _signature = _host;
  _signature << ':' << _port;
  if ((al= (String *) serveraliases->Find(_signature)))
    {
      delim= al->indexOf(':');
      // fprintf(stderr, "\nOld URL: %s->%s\n", (char *) _signature, (char *) *al);
      _host= al->sub(0,delim).get();
      sscanf((char*)al->sub(delim+1), "%d", &newport);
      _port= newport;
      // fprintf(stderr, "New URL: %s:%d\n", (char *) _host, _port);
    }
}

//*****************************************************************************
// void URL::constructURL()
// Constructs the _url member from everything else
// Also ensures the port number is correct for the service
//
void URL::constructURL()
{
    _url = _service;
    _url << ":";

    if (!(strcmp((char*)_service, "news") == 0 ||
	  strcmp((char*)_service, "mailto") == 0 ))
	_url << "//";

    if (strcmp((char*)_service, "file") != 0)
      {
	if (_user.length())
	  _url << _user << '@';
	_url << _host;
      }

    if (_port != DefaultPort())  // Different to the default port
      _url << ':' << _port;

    _url << _path;
}


int URL::DefaultPort()
{
   if (strcmp((char*)_service, "http") == 0)
      return 80;
   else if (strcmp((char*)_service, "https") == 0)
      return 443;
   else if (strcmp((char*)_service, "ftp") == 0)
      return 21;
   else if (strcmp((char*)_service, "gopher") == 0)
      return 70;
   else if (strcmp((char*)_service, "news") == 0)
      return NNTP_DEFAULT_PORT;
   else return 80;

}