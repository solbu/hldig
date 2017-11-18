//
// URL.cc
//
// URL: A URL parsing class, implementing as closely as possible the standard
//      laid out in RFC2396 (e.g. http://www.faqs.org/rfcs/rfc2396.html)
//      including support for multiple services. (schemes in the RFC)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: URL.cc,v 1.16 2004/06/04 08:51:01 angusgb Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "URL.h"
#include "QuotedStringList.h"
#include "Dictionary.h"
#include "HtConfiguration.h"
#include "StringMatch.h"
#include "StringList.h"
#include "HtURLRewriter.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

#include <sys/types.h>
#include <ctype.h>

#ifndef _MSC_VER /* _WIN32 */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#define NNTP_DEFAULT_PORT 119

static Dictionary  *slashCount = 0;

//*****************************************************************************
// URL::URL()
// Default Constructor
//
URL::URL()
: _url(0),
    _path(0),
    _service(0),
    _host(0),
    _port(0),
    _normal(0),
    _hopcount(0),
    _signature(0),
    _user(0)
{
}


//*****************************************************************************
// URL::URL(const URL& rhs)
// Copy constructor
//
URL::URL(const URL& rhs)
: _url(rhs._url),
    _path(rhs._path),
    _service(rhs._service),
    _host(rhs._host),
    _port(rhs._port),
    _normal(rhs._normal),
    _hopcount(rhs._hopcount),
    _signature(rhs._signature),
    _user(rhs._user)
{
}


//*****************************************************************************
// URL::URL(const String &nurl)
// Construct a URL from a String (obviously parses the string passed in)
// 
URL::URL(const String &nurl)
: _url(0),
    _path(0),
    _service(0),
    _host(0),
    _port(0),
    _normal(0),
    _hopcount(0),
    _signature(0),
    _user(0)
{
    parse(nurl);
}


//*****************************************************************************
// Assignment operator
const URL &URL::operator = (const URL &rhs)
{
  if (this == &rhs)
    return *this;

  // Copy the attributes
  _url = rhs._url;
  _path = rhs._path;
  _service = rhs._service;
  _host = rhs._host;
  _port = rhs._port;
  _normal = rhs._normal;
  _hopcount = rhs._hopcount;
  _signature = rhs._signature;
  _user = rhs._user;

  return *this;
}

//*****************************************************************************
// URL::URL(const String &url, const URL &parent)
//   Parse a reference given a parent url.  This is needed to resolve relative
//   references which do NOT have a full url.
//
URL::URL(const String &url, const URL &parent)
: _url(0),
    _path(0),
    _service(parent._service),
    _host(parent._host),
    _port(parent._port),
    _normal(parent._normal),
    _hopcount(parent._hopcount + 1), // Since this is one hop *after* the parent, we should account for this
    _signature(parent._signature),
    _user(parent._user)
{
  HtConfiguration* config= HtConfiguration::config();
    int  allowspace = config->Boolean("allow_space_in_url", 0);
    String      temp;
    const char *urp = url.get();
    while (*urp)
    {
  if (*urp == ' ' && temp.length() > 0 && allowspace)
  {
      // Replace space character with %20 if there's more non-space
      // characters to come...
      const char *s = urp+1;
      while (*s && isspace(*s))
    s++;
      if (*s)
    temp << "%20";
  }
  else if (!isspace(*urp))
      temp << *urp;
  urp++;
    }
    char* ref = temp;

    //
    // Strip any optional anchor from the reference.  If, however, the
    // reference contains CGI parameters after the anchor, the parameters
    // will be moved left to replace the anchor.  The overall effect is that
    // the anchor is removed.
    // Thanks goes to David Filiatrault <dwf@WebThreads.Com> for suggesting
    // this removal process.
    //
    char  *anchor = strchr(ref, '#');
    char  *params = strchr(ref, '?');
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
    char  *p = ref;
    while (isalpha(*p))  // Skip through the service portion
  p++;
    int  hasService = (*p == ':');
      // Why single out http?  Shouldn't others be the same?
  // Child URL of the form  https:/child  or  ftp:child  called "full"
  // How about using slashes()?
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
  String  fullref(parent._service);
  fullref << ':' << ref;
  parse((char*)fullref);
    }
    else
    {
  if (hasService)
      ref = p + 1;  // Relative URL, skip "http:"

  if (*ref == '/')
  {
      //
      // The reference is on the same server as the parent, but
      // an absolute path was given...
      //
      _path = ref;

            //
            // Get rid of loop-causing constructs in the path
            //
            normalizePath();
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

      //
      // Remove any leading "./" sequences which could get us into
      // recursive loops.
      //
      while (strncmp(ref, "./", 2) == 0)
    ref += 2;

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
    String  temp = _path;
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
// void URL::rewrite()
//
void URL::rewrite()
{
  if (HtURLRewriter::instance()->replace(_url) > 0)
    parse(_url.get());
}


//*****************************************************************************
// void URL::parse(const String &u)
//   Given a URL string, extract the service, host, port, and path from it.
//
void URL::parse(const String &u)
{
  HtConfiguration* config= HtConfiguration::config();
    int  allowspace = config->Boolean("allow_space_in_url", 0);
    String  temp;
    const char *urp = u.get();
    while (*urp)
    {
  if (*urp == ' ' && temp.length() > 0 && allowspace)
  {
      // Replace space character with %20 if there's more non-space
      // characters to come...
      const char *s = urp+1;
      while (*s && isspace(*s))
    s++;
      if (*s)
    temp << "%20";
  }
  else if (!isspace(*urp))
      temp << *urp;
  urp++;
    }
    char  *nurl = temp;

    //
    // Ignore any part of the URL that follows the '#' since this is just
    // an index into a document.
    //
    char  *p = strchr(nurl, '#');
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
  if (p)    // if non-NULL, skip (some) leading slashes in path
  {
      int i;
      for (i = slashes (_service); i > 0 && *p == '/'; i--)
    p++;
      if (i)  // if fewer slashes than specified for protocol don't
      // delete any. -> Backwards compatible (necessary??)
    p -= slashes (_service) - i;
  }
  _path = p;
  if (strcmp((char*)_service, "file") == 0 || slashes (_service) < 2)
    _host = "localhost";
    }
    else
    {
  p += 2;

  //
  // p now points to the host
  //
  char  *q = strchr(p, ':');
  char  *slash = strchr(p, '/');
    
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
        _path << strtok(p+1, "\n");  // _path is "/" - don't double
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
// Called from: URL(const String &url, const URL &parent)
//
void URL::normalizePath()
{
    //
    // Rewrite the path to be the minimal.
    // Remove "//", "/../" and "/./" components
    //
  HtConfiguration* config= HtConfiguration::config();

    int  i, limit;
    int  leadingdotdot = 0;
    String  newPath;
    int  pathend = _path.indexOf('?');  // Don't mess up query strings.
    if (pathend < 0)
        pathend = _path.length();

    //
    // get rid of "//" first, or "/foo//../" will become "/foo/" not "/"
    // Some database lookups interpret empty paths (// != /), so give
    // the use the option to turn this off.
    //
    if (!config->Boolean ("allow_double_slash"))
  while ((i = _path.indexOf("//")) >= 0 && i < pathend)
  {
      newPath = _path.sub(0, i).get();
      newPath << _path.sub(i + 1).get();
      _path = newPath;
      pathend = _path.indexOf('?');
      if (pathend < 0)
    pathend = _path.length();
  }

    //
    // Next get rid of redundant "/./".  This could cause infinite
    // loops.  Moreover, "/foo/./../" should become "/", not "/foo/"
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
        newPath = _path.sub(0, i+1).get();    // keep trailing slash
        newPath << _path.sub(i + 2).get();
        _path = newPath;
        pathend--;
    }

    //
    // Now that "empty" path components are gone, remove ("/../").
    //
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
            newPath = _path.sub(0, limit+1).get();  // keep trailing slash
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
    if (!config->Boolean("case_sensitive", 1))
      _path.lowercase();

    // And don't forget to remove index.html or similar file.
//    if (strcmp((char*)_service, "file") != 0)  (check is now internal)
  removeIndex(_path, _service);
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
// void URL::path(const String &newpath)
//
void URL::path(const String &newpath)
{
  HtConfiguration* config= HtConfiguration::config();
    _path = newpath;
    if (!config->Boolean("case_sensitive",1))
      _path.lowercase();
    constructURL();
}


//*****************************************************************************
// void URL::removeIndex(String &path, String &service)
//   Attempt to remove the remove_default_doc from the end of a URL path if
//   the service allows that.  (File, ftp don't.  Do others?)
//   This needs to be done to normalize the paths and make .../ the
//   same as .../index.html
// Called from: URL::normalize() from URL::signature()  [redundant?]
//     URL::normalizePath()
//
void URL::removeIndex(String &path, String &service)
{
  HtConfiguration* config= HtConfiguration::config();
    static StringMatch *defaultdoc = 0;

    if (strcmp((char*)_service, "file") == 0 ||
        strcmp((char*)_service, "ftp")  == 0)
  return;

    if (path.length() == 0 || strchr((char*)path, '?'))
  return;

    int filename = path.lastIndexOf('/') + 1;
    if (filename == 0)
        return;

    if (! defaultdoc)
    {
      StringList  l(config->Find("remove_default_doc"), " \t");
      defaultdoc = new StringMatch();
      defaultdoc->IgnoreCase();
      defaultdoc->Pattern(l.Join('|'));
    }
    int which, length;
    if (defaultdoc->hasPattern() &&
      defaultdoc->CompareWord((char*)path.sub(filename), which, length) &&
      filename+length == path.length())
  path.chop(path.length() - filename);
}


//*****************************************************************************
// void URL::normalize()
//   Make sure that URLs are always in the same format.
//
void URL::normalize()
{
  HtConfiguration* config= HtConfiguration::config();
    static int  hits = 0, misses = 0;

    if (_service.length() == 0 || _normal)
  return;

    
//  if (strcmp((char*)_service, "http") != 0)
    // if service specifies "doesn't specify an IP host", don't normalize it
    if (slashes (_service) != 2)
  return;

//    if (strcmp ((char*)_service, "http") == 0)  (check is now internal)
  removeIndex(_path, _service);

    //
    // Convert a hostname to an IP address
    //
    _host.lowercase();

    if (!config->Boolean("allow_virtual_hosts", 1))
    {
  static Dictionary  hostbyname;
  unsigned long    addr;
  struct hostent    *hp;

  String  *ip = (String *) hostbyname[_host];
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

  static Dictionary  machines;
  String      key;
  key << int(addr);
  String      *realname = (String *) machines[key];
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
// const String &URL::signature()
//   Return a string which uniquely identifies the server the current
//   URL is refering to.
//   This is the first portion of a url: service://user@host:port/
//   (in short this is the URL pointing to the root of this server)
//
const String &URL::signature()
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
  HtConfiguration* config= HtConfiguration::config();
  static Dictionary *serveraliases= 0;

  if (! serveraliases)
    {
      String l= config->Find("server_aliases");
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
    from.lowercase();
    if (from.indexOf(':') == -1)
      from.append(":80");
    to= new String(salias);
    to->lowercase();
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
  String serversig = _host;
  serversig << ':' << _port;
  if ((al= (String *) serveraliases->Find(serversig)))
    {
      delim= al->indexOf(':');
      // fprintf(stderr, "\nOld URL: %s->%s\n", (char *) serversig, (char *) *al);
      _host= al->sub(0,delim).get();
      sscanf((char*)al->sub(delim+1), "%d", &newport);
      _port= newport;
      // fprintf(stderr, "New URL: %s:%d\n", (char *) _host, _port);
    }
}

//*****************************************************************************
// int URL::slash(const String &protocol)
// Returns number of slashes folowing the service name for protocol
//
int
URL::slashes(const String &protocol)
{
    if (!slashCount)
    {
  HtConfiguration* config= HtConfiguration::config();
  slashCount = new Dictionary();

  slashCount->Add (String("mailto"), new String("0"));
  slashCount->Add (String("news"),   new String("0"));
  slashCount->Add (String("http"),   new String("2"));
  slashCount->Add (String("ftp"),    new String("2"));
  // file:///  has three, but the last counts as part of the path...
  slashCount->Add (String("file"),   new String("2"));
  
  QuotedStringList  qsl(config->Find("external_protocols"), " \t");
  String      from;
  int      i;
  int      sep,colon;

  for (i = 0; qsl[i]; i += 2)
  {
      from = qsl[i];
      sep = from.indexOf("->");
      if (sep != -1)
    from = from.sub(0, sep).get();  // "get" aids portability...

      colon = from.indexOf(":");
      // if service specified as "help:/" or "man:", note trailing slashes
      // Default is 2.
      if (colon != -1)
      {
    int i;
    char count [2];
    for (i = colon+1; from[i] == '/'; i++)
        ;
    count [0] = i - colon + '0' - 1;
    count [1] = '\0';
    from = from.sub(0,colon).get();
    slashCount->Add (from, new String (count));
      } else
    slashCount->Add (from, new String ("2"));
  }
    }
    
    // Default to two slashes for unknown protocols
    String *count = (String *)slashCount->Find(protocol);
    return count ? (count->get()[0] - '0') : 2;
}

//*****************************************************************************
// void URL::constructURL()
// Constructs the _url member from everything else
// Also ensures the port number is correct for the service
// Called from  URL::URL(const String &url, const URL &parent)
//    URL::parse(const String &u)
//    URL::path(const String &newpath)
//    URL::normalize()
//
void URL::constructURL()
{
    if (strcmp((char*)_service, "file") != 0 && _host.length() == 0) {
  _url = "";
  return;
    }

    _url = _service;
    _url << ":";

    // Add correct number of slashes after service name
    int i;
    for (i = slashes (_service); i > 0; i--)
    {
  _url << "/";
    }

    if (slashes (_service) == 2)  // services specifying a particular
    {          // IP host must begin "service://"
  if (strcmp((char*)_service, "file") != 0)
    {
      if (_user.length())
        _url << _user << '@';
      _url << _host;
    }

       if (_port != DefaultPort() && _port != 0)  // Different than the default port
    _url << ':' << _port;
    }

    _url << _path;
}


///////
   //    Get the default port for the recognised service
///////

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
   else if (strcmp((char*)_service, "file") == 0)
      return 0;
   else if (strcmp((char*)_service, "news") == 0)
      return NNTP_DEFAULT_PORT;
   else return 80;
}
