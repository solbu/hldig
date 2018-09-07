//
// cgi.h
//
// cgi: Parse cgi arguments and put them in a dictionary.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: cgi.h,v 1.4 2004/05/28 13:15:12 lha Exp $
//

#ifndef _cgi_h_
#define _cgi_h_

class Dictionary;

class cgi
{
public:
  cgi ();
  cgi (char *s);
   ~cgi ();

  const char *operator [] (const char *);
  const char *get (const char *);
  int exists (const char *);
  char *path ();

private:
    Dictionary * pairs;
  int query;
  void init (const char *s);
};

#endif
