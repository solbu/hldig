//
// ExternalTransport.cc
//
// ExternalTransport: Allows external programs to retrieve given URLs with
//                    unknown protocols.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExternalTransport.cc,v 1.1.2.1 1999/11/28 02:43:12 ghutchis Exp $
//

#include "ExternalTransport.h"
#include "htdig.h"
#include "QuotedStringList.h"
#include "URL.h"
#include "Dictionary.h"
#include "good_strtok.h"

#include <ctype.h>
#include <stdio.h>

static Dictionary	*handlers = 0;
static Dictionary	*toTypes = 0;
extern String		configFile;

//*****************************************************************************
// ExternalTransport::ExternalTransport(char *protocol)
//
ExternalTransport::ExternalTransport(char *protocol)
{
    if (canHandle(protocol))
    {
	_Handler = ((String *)handlers->Find(protocol))->get();
    }
    ExternalTransport::_Protocol = protocol;
    _Response = new ExternalTransport_Response;
}


//*****************************************************************************
// ExternalTransport::~ExternalTransport()
//
ExternalTransport::~ExternalTransport()
{
    if (_Response)
    {
	delete _Response;
    }
}


//*****************************************************************************
// int ExternalTransport::canHandle(char *protocol)
//
int
ExternalTransport::canHandle(char *protocol)
{
    if (!handlers)
    {
	handlers = new Dictionary();
	toTypes = new Dictionary();
	
	QuotedStringList	qsl(config["external_protocols"], " \t");
	String			from, to;
	int			i;
	int			sep;

	for (i = 0; qsl[i]; i += 2)
	{
	    from = qsl[i];
	    to = "";
	    sep = from.indexOf("->");
	    if (sep != -1)
	    {
		to = from.sub(sep+2).get();
		from = from.sub(0, sep).get();
	    }
	    handlers->Add(from, new String(qsl[i + 1]));
	    toTypes->Add(from, new String(to));
	}
    }
    return handlers->Exists(protocol);
}


//*****************************************************************************
// void ExternalTransport::SetConnection(URL *u)
//
void ExternalTransport::SetConnection (URL *u)
{
    // Grab the actual URL to pass to the handler
    _URL = *u;

    // OK, now call the parent method to make sure everything else is set up.
    Transport::SetConnection (u->host(), u->port());
}


//*****************************************************************************
// DocStatus ExternalTransport::Request()
//
Transport::DocStatus ExternalTransport::Request()
{
    //
    // Start the external handler, passing the URL quoted on the command-line
    //
    String	command = _Handler;
    command << ' ' << _Protocol << " \"" << _URL.get() << "\" " << configFile;

    FILE	*input = popen((char*)command, "r");
    
    // Set up a response for this request
    _Response->Reset();
    // We just accessed the document
    //    _Response->_access_time->SettoNow();
    
    
    // OK, now parse the stuff we got back from the handler...
    String	line;
    char	*token1;
    int		in_header = 1;

    while (in_header && readLine(input, line))
    {
	token1 = strtok(line, "\t");
	if (token1 == NULL)
	  {
	    token1 = "";
	    in_header = 0;
	    break;
	  }

	switch (*token1)
	 {
	    case 's':	// status code
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_status_code = atoi(token1);
		else
		  cerr<< "External transport error: expected status code in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
		
	    case 'r':	// status reason
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_reason_phrase = token1;
		else
		  cerr<< "External transport error: expected status reason in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
		
	    case 'm':	// modification time
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_modification_time= NewDate(token1);  // Hopefully we can grok it...
		else
		  cerr<< "External transport error: expected modification time in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
		
	    case 't':	// Content-Type
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_content_type = token1;
		else
		  cerr<< "External transport error: expected content-type in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
		
	    case 'l':	// Content-Length
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_content_length = atoi(token1);
		else
		  cerr<< "External transport error: expected content-length in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
	
	    case 'u':	// redirect target
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  _Response->_location = token1;
		else
		  cerr<< "External transport error: expected URL in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
		
	    default:
		  cerr<< "External transport error: unknown field in line "<<line<<"\n" << " URL: " << _URL.get() << "\n";
		break;
	}
    }

    // OK, now we read in the rest of the document as contents...
    _Response->_contents = 0;
    char        docBuffer[8192];
    int         bytesRead;

    while ((bytesRead = fread(docBuffer, 1, sizeof(docBuffer), input)) > 0)
      {
        if (debug > 2)
	  cout << "Read " << bytesRead << " from document\n";
        if (_Response->_contents.length() + bytesRead > _max_document_size)
            bytesRead = _max_document_size - _Response->_contents.length();
        _Response->_contents.append(docBuffer, bytesRead);
        if (_Response->_contents.length() >= _max_document_size)
            break;
      }
    _Response->_document_length = _Response->_contents.length();
    pclose(input);

    return GetDocumentStatus(_Response);
}


//*****************************************************************************
// private
// DocStatus ExternalTransport::GetDocumentStatus(ExternalTransport_Response *r)
//
Transport::DocStatus ExternalTransport::GetDocumentStatus(ExternalTransport_Response *r)
{ 
   // The default is 'not found' if we can't figure it out...
   DocStatus returnStatus = Document_not_found;
   int statuscode = r->GetStatusCode();

   if (statuscode == 200)
   { 
	    returnStatus = Document_ok;   // OK
   	    // Is it parsable?
   }
   
   else if (statuscode > 200 && statuscode < 300)
	    returnStatus = Document_ok;      	   	 // Successful 2xx
   else if (statuscode == 304)
	    returnStatus = Document_not_changed;   	 // Not modified
   else if (statuscode > 300 && statuscode < 400)
	    returnStatus = Document_redirect;      	 // Redirection 3xx
   else if (statuscode == 401)
	    returnStatus = Document_not_authorized;   // Unauthorized

   return returnStatus;
}


//*****************************************************************************
// private
// int ExternalTransport::readLine(FILE *in, String &line)
//
int
ExternalTransport::readLine(FILE *in, String &line)
{
    char	buffer[2048];
    int		length;
    
    line = 0;
    while (fgets(buffer, sizeof(buffer), in))
    {
	length = strlen(buffer);
	if (buffer[length - 1] == '\n')
	{
	    //
	    // A full line has been read.  Return it.
	    //
	    line << buffer;
	    line.chop('\n');
	    return 1;
	}
	else
	{
	    //
	    // Only a partial line was read.  Append it to the line
	    // and read some more.
	    //
	    line << buffer;
	}
    }
    return line.length() > 0;
}

