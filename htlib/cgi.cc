//
// cgi.cc
//
// Implementation of cgi
//
// $Log: cgi.cc,v $
// Revision 1.6  1999/06/16 13:48:12  grdetil
// Allow a query string to be passed as an argument.
//
// Revision 1.5  1999/01/20 18:08:30  ghutchis
// Call good_strtok with appropriate parameters (explicitly include NULL first
// parameter, second param is char, not char *).
//
// Revision 1.4  1998/11/15 02:44:23  ghutchis
//
// Declared loop int variable. (to simplify frost.com merge)
//
// Revision 1.3  1998/05/26 03:58:11  turtle
// Got rid of compiler warnings.
//
// Revision 1.2  1997/03/24 04:33:23  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: cgi.cc,v 1.6 1999/06/16 13:48:12 grdetil Exp $";
#endif

#include "cgi.h"
#include "htString.h"
#include <Dictionary.h>
#include <good_strtok.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>
#include <StringList.h>
#include <URL.h>


//*****************************************************************************
// cgi::cgi()
//
cgi::cgi()
{
	init("");
}


//*****************************************************************************
// cgi::cgi(char *s)
//
cgi::cgi(char *s)
{
	init(s);
}


//*****************************************************************************
// void cgi::init(char *s)
//
void
cgi::init(char *s)
{
	pairs = new Dictionary;

	int i;
	String	method(getenv("REQUEST_METHOD"));

	if ((!s || !*s) && method.length() == 0)
	{
		//
		// Interactive mode
		//
		query = 1;
		return;
	}
	query = 0;
	String	results;

	if (s && *s && method.length() == 0)
	{
		results = s;
	}
	else if (strcmp(method, "GET") == 0)
	{
		results = getenv("QUERY_STRING");
	}
	else
	{
		int		n;
		char	*buf;
		
		n = atoi(getenv("CONTENT_LENGTH"));
		buf = new char[n + 1];
		read(0, buf, n);
		buf[n] = '\0';
		results = buf;
		delete buf;
	}

	//
	// Now we need to split the line up into name/value pairs
	//
	StringList	list(results, '&');
	
	//
	// Each name/value pair now needs to be added to the dictionary
	//
	for (i = 0; i < list.Count(); i++)
	{
		char	*name = good_strtok(list[i], '=');
		String	value(good_strtok(NULL, '\n'));
		value.replace('+', ' ');
		decodeURL(value);
		String	*str = (String *) pairs->Find(name);
		if (str)
		{
			//
			// Entry was already there.  Append it to the string.
			//
			str->append('\001');
			str->append(value);
		}
		else
		{
			//
			// New entry.  Add a new string
			//
			pairs->Add(name, new String(value));
		}
	}
}


//*****************************************************************************
// cgi::~cgi()
//
cgi::~cgi()
{
	delete pairs;
}


//*****************************************************************************
// char *cgi::operator [] (char *name)
//
char *cgi::operator [] (char *name)
{
	return get(name);
}


//*****************************************************************************
// char *cgi::get(char *name)
//
char *cgi::get(char *name)
{
	String	*str = (String *) (*pairs)[name];
	if (str)
		return str->get();
	else
	{
		if (query)
		{
			char	buffer[1000];
			cerr << "Enter value for " << name << ": ";
			cin.getline(buffer, sizeof(buffer));
			pairs->Add(name, new String(buffer));
			str = (String *) (*pairs)[name];
			return str->get();
		}
		return 0;
	}
}


//*****************************************************************************
// int cgi::exists(char *name)
//
int
cgi::exists(char *name)
{
	return pairs->Exists(name);
}

//*****************************************************************************
// char *cgi::path()
//
char *cgi::path()
{
	static char	buffer[1000] = "";

	if (query)
	{
		if (*buffer)
			return buffer;
		cerr << "Enter PATH_INFO: ";
		cin.getline(buffer, sizeof(buffer));
		return buffer;
	}
	return getenv("PATH_INFO");
}


