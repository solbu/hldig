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
// $Id: Document.h,v 1.6.2.1 2000/02/15 22:42:20 grdetil Exp $
//
//
#ifndef _Document_h_
#define _Document_h_

#include "Parsable.h"
#include "Object.h"
#include "URL.h"
#include "htString.h"
#include "StringList.h"
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
	Document_not_authorized,
	Document_not_local
    };
    DocStatus			RetrieveHTTP(time_t date);
    DocStatus			RetrieveLocal(time_t date, StringList *filenames);

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
    int				contentLength;
    int				document_length;
    time_t			modtime;
    int				max_doc_size;

    int				readHeader(Connection &);
    time_t			getdate(char *datestring);
    int				UseProxy();
};

#endif


