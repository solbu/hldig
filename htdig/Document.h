//
// Document.h
//
// Document: This class holds everything there is to know about a document.
//           The actual contents of the document may or may not be present at
//           all times for memory conservation reasons.
//           The document can be told to retrieve its contents.  This is done
//           with the Retrieve call.  In case the retrieval causes a 
//           redirect, the link is followed, but this process is done 
//           only once (to prevent loops.) If the redirect didn't 
//           work, Document_not_found is returned.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Document.h,v 1.10.2.1 1999/10/13 11:55:21 angus Exp $
//
//
#ifndef _Document_h_
#define _Document_h_

#include "Parsable.h"
#include "Object.h"
#include "URL.h"
#include "htString.h"
#include "Transport.h"
#include "HtHTTP.h"

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
    int				Length()	  {return document_length;}
    char			*Contents()	  {return contents;}
    void			Contents(char *s) {contents = s; document_length = contents.length();}

    //
    // In case the retrieval process went through a redirect process,
    // the new url can be gotten using the following call
    //
    char			*Redirected()		{return redirected_to;}
    URL				*Url()			{return url;}
    void			Url(char *url);
    void			Referer(char *url);
    time_t			ModTime()		{return modtime.GetTime_t();}

    Transport::DocStatus	Retrieve(HtDateTime date);
    Transport::DocStatus	RetrieveLocal(HtDateTime date, String filename);

    //
    // Return an appropriate parsable object for the document type.
    //
    Parsable			*getParsable();

    //
    // Set the username and password to be used in any requests
    //
    void			setUsernamePassword(const char *credentials)
                                          { authorization = credentials;}
	
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
    URL				*referer;
    String			contents;
    String			redirected_to;
    String			contentType;
    String			authorization;
    int				contentLength;
    int				document_length;
    HtDateTime			modtime;
    int				max_doc_size;

    int				UseProxy();

    Transport			*transportConnect;
    HtHTTP			*HTTPConnect;
};

#endif


