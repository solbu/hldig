//
// cgi.h
//
// cgi: Parse cgi arguments and put them in a dictionary.
//
// $Id: cgi.h,v 1.3 1999/09/08 14:42:29 loic Exp $
//
// $Log: cgi.h,v $
// Revision 1.3  1999/09/08 14:42:29  loic
// update comments
//
// Revision 1.2  1999/06/16 13:48:12  grdetil
// Allow a query string to be passed as an argument.
//
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
					cgi(char *s);
					~cgi();

	char			*operator [] (char *);
	char			*get(char *);
	int				exists(char *);
	char			*path();

private:
	Dictionary		*pairs;
	int				query;
	void				init(char *s);
};

#endif


