//
// Document.h
//
// This class holds everything there is to know about a document.
// The actual contents of the document may or may not be present at
// all times for memory conservation reasons.
// The document can be told to retrieve its contents.  This is done
// with the Retrieve call.  In case the retrieval causes a redirect, the
// link is followed, but this process is done only once (to prevent loops.)
// If the redirect didn't work, Document_not_found is returned.
//
// $Id: Document.h,v 1.3 1997/03/24 04:33:15 turtle Exp $
//
// $Log: Document.h,v $
// Revision 1.3  1997/03/24 04:33:15  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.2  1997/02/10 17:32:38  turtle
// Applied AIX specific patches supplied by Lars-Owe Ivarsson
// <lars-owe.ivarsson@its.uu.se>
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _Document_h_
#define _Document_h_

#include "Parsable.h"
#include <Object.h>
#include <URL.h>
#include <htString.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

class Connection;


class Document : public Object
{
public:
    //
    // Construction/Destruction
    //
    Document(char *url = 0, int max_size = 0);
    ~Document();

    //
    // Interface to the document.
    //
    void			Reset();
    int				Length()		{return document_length;}
    char			*Contents()		{return contents;}
    void			Contents(char *s) {contents = s; document_length = contents.length();}

    //
    // In case the retrieval process went through a redirect process,
    // the new url can be gotten using the following call
    //
    char			*Redirected()	{return redirected_to;}
    URL				*Url()			{return url;}
    void			Url(char *url);
    void			Referer(char *url)	{referer = url;}

    time_t			ModTime()		{return modtime;}

    //
    // If the contents is not available, retrieve it, but only if it
    // is newer than the given date string.  (Format is that of the
    // URC) Return values:
    //
    enum DocStatus
    {
	Document_ok,
	Document_not_changed,
	Document_not_found,
	Document_not_html,
	Document_redirect,
	Document_no_server,
	Document_no_host,
      Document_not_authorized
    };
    DocStatus			Retrieve(time_t date);

    //
    // Return an appropriate parsable object for the document type.
    //
    Parsable			*getParsable();

    //
    // Set the username and password to be used in the HTTP request
    //
    void			setUsernamePassword(char *credentials);
	
private:
    enum
    {
	Header_ok,
	Header_not_found,
	Header_not_changed,
	Header_redirect,
	Header_not_text,
      Header_not_authorized
    };

    URL				*url;
    URL				*proxy;
    String			contents;
    String			redirected_to;
    String			contentType;
    String			authorization;
    String			referer;
    int				document_length;
    time_t			modtime;
    int				max_doc_size;

    int				readHeader(Connection &);
    time_t			getdate(char *datestring);
};

#endif


