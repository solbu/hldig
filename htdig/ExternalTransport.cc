//
// ExternalTransport.cc
//
// ExternalTransport: Allows external programs to retrieve given URLs with
//                    unknown protocols.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExternalTransport.cc,v 1.5 2002/10/27 15:19:26 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "ExternalTransport.h"
#include "htdig.h"
#include "QuotedStringList.h"
#include "URL.h"
#include "Dictionary.h"
#include "good_strtok.h"

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef HAVE_WAIT_H
#include <wait.h>
#elif HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "defaults.h"

static Dictionary	*handlers = 0;
static Dictionary	*toTypes = 0;
extern String		configFile;

//*****************************************************************************
// ExternalTransport::ExternalTransport(char *protocol)
//
ExternalTransport::ExternalTransport(const String &protocol)
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
// int ExternalTransport::canHandle(const String &protocol)
//
int
ExternalTransport::canHandle(const String &protocol)
{
	HtConfiguration* config= HtConfiguration::config();
    if (!handlers)
    {
	handlers = new Dictionary();
	toTypes = new Dictionary();
	
	QuotedStringList	qsl(config->Find("external_protocols"), " \t");
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

	    // Recognise service specified as "https://" rather than "https"
	    sep = from.indexOf(":");
	    if (sep != -1)
		from = from.sub(0, sep).get();

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
    // Start the external handler, passing the protocol, URL and config file
    // as command arguments
    //
    StringList	hargs(_Handler);
    char   **handlargs = new char * [hargs.Count() + 5];
    int    argi;
    for (argi = 0; argi < hargs.Count(); argi++)
	handlargs[argi] = (char *)hargs[argi];
    handlargs[argi++] = _Protocol.get();
    handlargs[argi++] = (char *)_URL.get().get();
    handlargs[argi++] = configFile.get();
    handlargs[argi++] = 0;

    int    stdout_pipe[2];
    int	   fork_result = -1;
    int	   fork_try;

    if (pipe(stdout_pipe) == -1)
    {
      if (debug)
	cerr << "External transport error: Can't create pipe!" << endl;
      delete [] handlargs;
      return GetDocumentStatus(_Response);
    }

    for (fork_try = 4; --fork_try >= 0;)
    {
      fork_result = fork(); // Fork so we can execute in the child process
      if (fork_result != -1)
	break;
      if (fork_try)
	sleep(3);
    }
    if (fork_result == -1)
    {
      if (debug)
	cerr << "Fork Failure in ExternalTransport" << endl;
      delete [] handlargs;
      return GetDocumentStatus(_Response);
    }

    if (fork_result == 0) // Child process
    {
	close(STDOUT_FILENO); // Close handle STDOUT to replace with pipe
	dup(stdout_pipe[1]);
	close(stdout_pipe[0]);
	close(stdout_pipe[1]);
	// not really necessary, and may pose Cygwin incompatibility...
	//close(STDIN_FILENO); // Close STDIN to replace with null dev.
	//open("/dev/null", O_RDONLY);

	// Call External Transport Handler
	execv(handlargs[0], handlargs);

	exit(EXIT_FAILURE);
    }

    // Parent Process
    delete [] handlargs;
    close(stdout_pipe[1]); // Close STDOUT for writing
    FILE *input = fdopen(stdout_pipe[0], "r");
    if (input == NULL)
    {
      if (debug)
	cerr << "Fdopen Failure in ExternalTransport" << endl;
      return GetDocumentStatus(_Response);
    }
    
    // Set up a response for this request
    _Response->Reset();
    // We just accessed the document
    _Response->_access_time = new HtDateTime();
    _Response->_access_time->SettoNow();
    
    
    // OK, now parse the stuff we got back from the handler...
    String	line;
    char	*token1;
    int		in_header = 1;

    while (in_header && readLine(input, line))
    {
	line.chop('\r');
	if (line.length() > 0 && debug > 2)
	    cout << "Header line: " << line << endl;
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
    fclose(input);
    // close(stdout_pipe[0]); // This is closed for us by the fclose()

    int rpid, status;
    while ((rpid = wait(&status)) != fork_result && rpid != -1)
	;

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

