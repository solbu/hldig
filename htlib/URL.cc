//
// URL.cc
//
// Implementation of URL
//
//
#if RELEASE
static char RCSid[] = "$Id: URL.cc,v 1.18.2.10 2001/10/12 20:04:30 grdetil Exp $";
#endif

#include "URL.h"
#include "Dictionary.h"
#include "Configuration.h"
#include "StringMatch.h"
#include "StringList.h"
#include "HtURLRewriter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

extern Configuration	config;

//*****************************************************************************
// URL::URL()
//
URL::URL()
{
    _normal = 0;
    _port = 80;
    _hopcount = 0;
}


//*****************************************************************************
// URL::URL(URL &nurl)
//
URL::URL(URL &nurl)
{
    _service = nurl._service;
    _host = nurl._host;
    _port = nurl._port;
    _url = nurl._url;
    _path = nurl._path;
    _normal = nurl._normal;
    _signature = nurl._signature;
    _hopcount = nurl._hopcount;
}


//*****************************************************************************
// URL::URL(char *nurl)
//   Parse a URL.  The general format is:
//
//		<service>://<host>[:<port>]/<path>
//
URL::URL(char *nurl)
{
    _port = 80;
    _normal = 0;
    _hopcount = 0;
    parse(nurl);
}


//*****************************************************************************
// URL::URL(char *ref, URL &parent)
//   Parse a reference given a parent url.  This is needed to resolve relative
//   references which do NOT have a full url.
//
URL::URL(char *ref, URL &parent)
{
    String	temp(ref);
    temp.remove(" \r\n\t");
    ref = temp;

    _host = parent._host;
    _port = parent._port;
    _service = parent._service;
    _normal = parent._normal;
    _hopcount = 0;

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
    // we just want to use the base URL.
    //
    if (!*ref)
    {
	_service = parent._service;
	_host = parent._host;
	_port = parent._port;
	_url = parent._url;
	_path = parent._path;
	_normal = parent._normal;
	_signature = parent._signature;
	_hopcount = parent._hopcount;
	return;
    }

    char	*p = ref;
    while (isalpha(*p))
	p++;
    int	hasService = (*p == ':');
    if ((hasService && ((strncmp(ref, "http://", 7) == 0) ||
		       (strncmp(ref, "http:", 5) != 0)))
	|| strncmp(ref, "//", 2) == 0)
    {
	//
	// No need to look at the parent url since this is a complete url...
	//
	parse(ref);
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
		p = strrchr(temp, '/');
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
	}

        //
        // Get rid of loop-causing constructs in the path
        //
        normalizePath();
    }
	
    //
    // Build the url.  (Note, the host name has NOT been normalized!)
    //
    _url = _service;
    _url << ":";
    if (_host.length())
	_url << "//" << _host;
    if (_port != 80 && strcmp(_service, "http") == 0)
	_url << ':' << _port;
    _url << _path;
}

void URL::rewrite()
{
      String _old = _url.get();
      HtURLRewriter::instance()->Replace(_url);
      if (strcmp(_old.get(), _url.get()) != 0)
              parse(_url.get());
}

//*****************************************************************************
// void URL::parse(char *u)
//   Given a URL string, extract the service, host, port, and path from it.
//
void URL::parse(char *u)
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
	
    //
    // Extract the host
    //
    if (!p || strncmp(p, "//", 2) != 0)
    {
	_host = 0;
	_port = 0;
	_url = 0;
	_path = p;
	_normal = 1;
	return;
    }
    else
	p += 2;

    //
    // p now points to the host
    //
    char	*q = strchr(p, ':');
    char	*slash = strchr(p, '/');
    
    if (q && ((slash && slash > q) || !slash))
    {
	_host = strtok(p, ":");
	p = strtok(0, "/");
	if (p)
	    _port = atoi(p);
	if (!p || _port <= 0)
	    _port = 80;
    }
    else
    {
	_host = strtok(p, "/");
	_host.chop(" \t");
	_port = 80;
    }

    //
    // The rest of the input string is the path.
    //
    _path = "/";
    _path << strtok(0, "\n");

    //
    // Get rid of loop-causing constructs in the path
    //
    normalizePath();

    //
    // Build the url.  (Note, the host name has NOT been normalized!)
    //
    _url = _service;
    _url << "://" << _host;
    if (_port != 80)
	_url << ':' << _port;
    _url << _path;
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
    int	pathend = _path.indexOf('?');	// Don't mess up query strings.
    if (pathend < 0)
        pathend = _path.length();
    while ((i = _path.indexOf("/../")) >= 0 && i < pathend)
    {
        if ((limit = _path.lastIndexOf('/', i - 1)) >= 0)
        {
            String	newPath;
            newPath << _path.sub(0, limit).get();
            newPath << _path.sub(i + 3).get();
            _path = newPath;
        }
        else
        {
            _path = _path.sub(i + 3).get();
        }
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }
    if ((i = _path.indexOf("/..")) >= 0 && i == pathend-3)
    {
        String	newPath;
        if ((limit = _path.lastIndexOf('/', i - 1)) >= 0)
            newPath << _path.sub(0, limit+1).get();	// keep trailing slash
        if (newPath.length() == 0)
            newPath << '/';
        newPath << _path.sub(i + 3).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }

    //
    // Also get rid of redundent "/./".  This could cause infinite
    // loops.
    //
    while ((i = _path.indexOf("/./")) >= 0 && i < pathend)
    {
        String	newPath;
        newPath << _path.sub(0, i).get();
        newPath << _path.sub(i + 2).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }
    if ((i = _path.indexOf("/.")) >= 0 && i == pathend-2)
    {
        String	newPath;
        newPath << _path.sub(0, i+1).get();		// keep trailing slash
        newPath << _path.sub(i + 2).get();
        _path = newPath;
        pathend--;
    }

    //
    // Furthermore, get rid of "//".  This could also cause loops
    //
    while ((i = _path.indexOf("//")) >= 0 && i < pathend)
    {
        String  newPath;
        newPath << _path.sub(0, i).get();
        newPath << _path.sub(i + 1).get();
        _path = newPath;
        pathend = _path.indexOf('?');
        if (pathend < 0)
            pathend = _path.length();
    }

    // Finally change all "%7E" to "~" for sanity
    while ((i = _path.indexOf("%7E")) >= 0 && i < pathend)
      {
        String  newPath;
        newPath << _path.sub(0, i).get();
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
}

//*****************************************************************************
// void URL::dump()
//
void URL::dump()
{
    printf("service = '%s'\n", _service.get());
    printf("host = '%s'\n", _host.get());
    printf("port = %d\n", _port);
    printf("url = '%s'\n", _url.get());
    printf("path = '%s'\n", _path.get());
}


//*****************************************************************************
// void URL::path(char *newpath)
//
void URL::path(char *newpath)
{
    _path = newpath;
    if (!config.Boolean("case_sensitive", 1))
      _path.lowercase();
    _url = _service;
    _url << "://" << _host;
    if (_port != 80)
	_url << ':' << _port;
    _url << _path;
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

    if (path.length() == 0 || strchr(path, '?'))
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
      l.Release();
    }
    int which, length;
    if (defaultdoc->hasPattern() &&
	    defaultdoc->CompareWord(path.sub(filename), which, length) &&
	    filename+length == path.length())
	path.chop(path.length() - filename);
}


//*****************************************************************************
// void URL::normalize()
//   Make sure that http URLs are always in the same format.
//
void URL::normalize()
{
    static int	hits = 0, misses = 0;

    if (_service.length() == 0 || _normal)
	return;

    if (strcmp(_service, "http") != 0)
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
    _url = _service;
    _url << ":";
    if (_host.length())
	_url << "//" << _host;
    if (_port != 80 && strcmp(_service, "http") == 0)
	_url << ':' << _port;
    _url << _path;
    _normal = 1;
    _signature = 0;
}


//*****************************************************************************
// char *URL::signature()
//   Return a string which uniquely identifies the server the current
//   URL is refering to.
//   This is just the string containing the host and port.
//
char *URL::signature()
{
    if (_signature.length())
	return _signature;

    if (!_normal)
	normalize();
    _signature = _host;
    _signature << ':' << _port;
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
	  // cout << "Alias: " << p << "->" << salias << "\n";
	  // printf ("Alias: %s->%s\n", p, salias);
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
      // printf("%s->%s\n", (char *) _signature, (char *) *al);
      _host= al->sub(0,delim).get();
      sscanf(al->sub(delim+1), "%d", &newport);
      _port= newport;
      // printf("\nNewer URL: %s:%d\n", (char *) _host, _port);
    }
}
