//
// cgi.h
//
// cgi: Parse cgi arguments and put them in a dictionary.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: cgi.h,v 1.3 2003/06/24 20:05:44 nealr Exp $
//

#ifndef _cgi_h_
#define _cgi_h_

class Dictionary;

class cgi
{
public:
	cgi();
	cgi(char *s);
	~cgi();

	char			*operator [] (char *);
	char			*get(char *);
	int			exists(char *);
	char			*path();

private:
	Dictionary		*pairs;
	int			query;
	void			init(char *s);
};

#endif


