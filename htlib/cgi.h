//
// cgi.h
//
// $Id: cgi.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: cgi.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _cgi_h_
#define _cgi_h_

class Dictionary;

class cgi
{
public:
					cgi();
					~cgi();

	char			*operator [] (char *);
	char			*get(char *);
	int				exists(char *);
	char			*path();

private:
	Dictionary		*pairs;
	int				query;
};

#endif


