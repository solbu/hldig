//
// Document.cc
//
// Implementation of Document
//
// $Log: Document.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Document.cc,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $";
#endif

#include <signal.h>
#include "Document.h"
#include "Connection.h"
#include "htdig.h"
#include "HTML.h"
#include "Plaintext.h"
#include "Postscript.h"
#include "ExternalParser.h"

#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif

static Connection	*current_connection;


//*****************************************************************************
// Document::Document(char *u)
//   Initialize with the given url as the location for this document.
//   If the max_size is given, use that for size, otherwise use the
//   config value.
//
Document::Document(char *u, int max_size)
{
    url = 0;
    proxy = 0;
    referer = 0;

    if (max_size > 0)
	max_doc_size = max_size;
    else
	max_doc_size = config.Value("max_doc_size");
	
    char	*proxyURL = config["http_proxy"];
    if (proxyURL && *proxyURL)
    {
	proxy = new URL(proxyURL);
	proxy->normalize();
    }

    contents.allocate(max_doc_size + 100);
    contentType = "";
    if (u)
    {
	Url(u);
    }
}


//*****************************************************************************
// Document::~Document()
//
Document::~Document()
{
    delete url;
//	delete proxy;
#if MEM_DEBUG
    char *p = new char;
    cout << "==== Document deleted: " << this << " new at " <<
	((void *) p) << endl;
    delete p;
#endif
}


//*****************************************************************************
// void Document::Reset()
//   Restore the Document object to an initial state.
//   We will not reset the authorization information since it can be reused.
//
void
Document::Reset()
{
    contentType = "";
    delete url;
    url = 0;
    referer = 0;
    modtime = 0;

    contents = 0;
    redirected_to = 0;
    authorization = 0;
}


//*****************************************************************************
// void Document::setUsernamePassword(char *credentials)
//
void
Document::setUsernamePassword(char *credentials)
{
    static char	tbl[64] =
    {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
    };
    authorization = 0;
    char	*p;
    int		n = strlen(credentials);
    int		ch;

    for (p = credentials; n > 2; n -= 3, p += 3)
    {
	ch = *p >> 2;
	authorization << tbl[ch & 077];
	ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
	authorization << tbl[ch & 077];
	ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
	authorization << tbl[ch & 077];
	ch = p[2] & 077;
	authorization << tbl[ch & 077];
    }

    if (n != 0)
    {
	char c1 = *p;
	char c2 = n == 1 ? 0 : p[1];

	ch = c1 >> 2;
	authorization << tbl[ch & 077];

	ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
	authorization << tbl[ch & 077];

	if (n == 1)
	    authorization << '=';
	else
        {
	    ch = (c2 << 2) & 074;
	    authorization << tbl[ch & 077];
        }
	authorization << '=';
    }
}


//*****************************************************************************
// void Document::Url(char *u)
//   Set the URL for this document
//
void
Document::Url(char *u)
{
    delete url;
    url = new URL(u);
}


//*****************************************************************************
// time_t Document::getdate(char *datestring)
//   Convert a RFC850 date string into a time value
//
time_t
Document::getdate(char *datestring)
{
    String	d = datestring;
    d.replace('-', ' ');

    time_t	now = time(0);
#if HAVE_LOCALTIME
    struct tm	*tm = localtime(&now);
#else
    struct tm	*tm = gmtime(&now);
#endif
#if HAVE_TM_GMTOFF
    tm->tm_zone = "GMT";
    tm->tm_gmtoff = 0;
#endif
    mystrptime(d.get(), "%a, %d %b %Y %T", tm);

    if (tm->tm_year < 0)
	tm->tm_year += 1900;

    if (debug > 2)
    {
	cout << "Translated " << d << " to ";
	char	buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T", tm);
	cout << buffer << " (" << tm->tm_year << ")" << endl;
    }
#if HAVE_MKTIME
    return mktime(tm);
#else
    return timelocal(tm);
#endif
}


static void
timeout()
{
    if (debug > 1)
	printf(" Timeout\n");
    current_connection->stop_io();

    struct sigaction    sa;
    sa.sa_handler = (SIGNAL_HANDLER) timeout;
    sigemptyset ((sigset_t *) &sa.sa_mask);
    sigaddset ((sigset_t *) &sa.sa_mask, SIGALRM);
#if defined(SA_INTERRUPT)
    sa.sa_flags = SA_INTERRUPT;
#else
    sa.sa_flags = 0;
#endif
    sigaction(SIGALRM, &sa, 0);
    alarm(config.Value("timeout"));
}


//*****************************************************************************
// DocStatus Document::Retrieve(time_t date)
//   Attempt to retrieve the document pointed to by our internal URL
//
Document::DocStatus
Document::Retrieve(time_t date)
{
    Connection	c;
    if (c.open() == NOTOK)
	return Document_not_found;

    if (proxy)
    {
	if (c.assign_port(proxy->port()) == NOTOK)
	    return Document_not_found;
	if (c.assign_server(proxy->host()) == NOTOK)
	    return Document_no_host;
    }
    else
    {
	if (c.assign_port(url->port()) == NOTOK)
	    return Document_not_found;
	if (c.assign_server(url->host()) == NOTOK)
	    return Document_no_host;
    }
	
    if (c.connect(1) == NOTOK)
    {
	if (debug > 1)
	{
	    cout << "Unable to build connection with " << url->host() << ':' << url->port() << endl;
	}
	return Document_no_server;
    }

    current_connection = &c;

    //
    // Construct and send the request to the server
    //
    String        command = "GET ";

    if (proxy)
    {
	command << url->get() << " HTTP/1.0\r\n";
    }
    else
    {
	command << url->path() << " HTTP/1.0\r\n";
    }
    command << "User-Agent: htdig/" << HTDIG_VERSION <<
	" (" <<	config["maintainer"] << ")\r\n";

    //
    // If a referer was provided, we'll send that as well.
    //
    if (referer.length())
    {
	command << "Referer: " << referer << "\r\n";
    }
	
    //
    // If a date was provided, we'll use that in the special
    // 'If-modified-since' URC header.
    //
    if (date > 0)
    {
	struct tm	*tm = gmtime(&date);
	char		buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %h %Y %T GMT", tm);
	command << "If-Modified-Since: " << buffer << "\r\n";
    }

    //
    // If authorization was provided, send it.  This will happen regardless of
    // whether the server needs it or not.  Oh well.
    //
    if (authorization.length())
    {
	command << "Authorization: Basic " << authorization << "\r\n";
    }

    //
    // Finally we can commit the request by sending a blank line.
    //
    command << "\r\n";

    if (debug > 2)
	cout << "Retrieval command for " << url->get() << ": " << command;

    c.write(command);

    //
    // Setup a timeout for the connection
    //
    struct sigaction    sa;
    sa.sa_handler = (SIGNAL_HANDLER) timeout;
    sigemptyset ((sigset_t *) &sa.sa_mask);
    sigaddset ((sigset_t *) &sa.sa_mask, SIGALRM);
#if defined(SA_INTERRUPT)
    sa.sa_flags = SA_INTERRUPT;
#else
    sa.sa_flags = 0;
#endif
    sigaction(SIGALRM, &sa, 0);
    int		timeout_interval = config.Value("timeout");
    alarm(timeout_interval);

    switch (readHeader(c))
    {
	case Header_ok:
	    break;
	case Header_not_changed:
	    return Document_not_changed;
	case Header_not_found:
	    return Document_not_found;
	case Header_redirect:
	    return Document_redirect;
	case Header_not_text:
	    return Document_not_html;
	case Header_not_authorized:
	    return Document_not_authorized;
    }

    //
    // Read in the document itself
    //
    contents = 0;
    char	docBuffer[8192];
    int		bytesRead;

    if (debug < 2)
	alarm(timeout_interval);
    while ((bytesRead = c.read(docBuffer, sizeof(docBuffer))) > 0)
    {
	if (debug > 2)
	    cout << "Read " << bytesRead << " from document\n";
	if (contents.length() + bytesRead > max_doc_size)
	    break;
	contents.append(docBuffer, bytesRead);
	if (debug < 2)
	    alarm(timeout_interval);
    }
    c.close();
    alarm(0);
    document_length = contents.length();

    if (debug > 2)
	cout << "Read a total of " << document_length << " bytes\n";
    return Document_ok;
}


//*****************************************************************************
// int Document::readHeader(Connection &c)
//   Read and interpret the header of the document
//
int
Document::readHeader(Connection &c)
{
    String	line;
    int		inHeader = 1;
    int		returnStatus = Header_not_found;
	
    modtime = 0;

    while (inHeader)
    {
	c.read_line(line, "\n");
	line.chop('\r');
	if (debug > 2)
	    cout << "Header line: " << line << endl;
	if (line.length() == 0)
	    inHeader = 0;
	else
	{
	    if (strncmp(line, "HTTP/", 5) == 0)
	    {
		//
		// Found the status line.  This will determine if we
		// continue or not
		//
		strtok(line, " ");
		char	*status = strtok(0, " ");
		if (status && strcmp(status, "200") == 0)
		{
		    returnStatus = Header_ok;
		}
		else if (status && strcmp(status, "302") == 0)
		{
		    returnStatus = Header_redirect;
		}
		else if (status && strcmp(status, "304") == 0)
		{
		    returnStatus = Header_not_changed;
		}
		else if (status && strcmp(status, "401") == 0)
		{
		    returnStatus = Header_not_authorized;
		}
	    }
	    else if (mystrncasecmp(line, "last-modified:", 14) == 0)
	    {
		strtok(line, " \t");
		modtime = getdate(strtok(0, "\n\t"));
	    }
	    else if (mystrncasecmp(line, "content-type:", 13) == 0)
	    {
		strtok(line, " \t");
		char	*token = strtok(0, "\n\t");
				
		if (mystrncasecmp("text/", token, 5) != 0 &&
		    mystrncasecmp("application/postscript", token, 22) != 0)
		    return Header_not_text;
		contentType = token;
	    }
	    else if (mystrncasecmp(line, "location:", 9) == 0)
	    {
		strtok(line, " \t");
		redirected_to = strtok(0, "\r\n \t");
	    }
	}
    }
    if (debug > 2)
	cout << "returnStatus = " << returnStatus << endl;
    return returnStatus;
}


//*****************************************************************************
// Parsable *Document::getParsable()
//   Given the content-type of a document, returns a document parser.
//   This will first look through the list of user supplied parsers and
//   then at our (limited) builtin list of parsers.  The user supplied
//   parsers are external programs that will be used.
//
Parsable *
Document::getParsable()
{
    static HTML			*html = 0;
    static Plaintext		*plaintext = 0;
    static Postscript		*postscript = 0;
    static ExternalParser	*externalParser = 0;
    
    Parsable	*parsable = 0;

    if (ExternalParser::canParse(contentType))
    {
	if (externalParser)
	{
	    delete externalParser;
	}
	externalParser = new ExternalParser(contentType);
	parsable = externalParser;
    }
    else if (mystrncasecmp(contentType, "text/html", 9) == 0)
    {
	if (!html)
	    html = new HTML();
	parsable = html;
    }
    else if (mystrncasecmp(contentType, "text/plain", 10) == 0)
    {
	if (!plaintext)
	    plaintext = new Plaintext();
	parsable = plaintext;
    }
    else if (mystrncasecmp(contentType, "application/postscript", 22) == 0)
    {
	if (!postscript)
	    postscript = new Postscript();
	parsable = postscript;
    }
    else
    {
	if (!plaintext)
	    plaintext = new Plaintext();
	parsable = plaintext;
	if (debug)
	{
	    cout << '"' << contentType <<
		"\" not a recognized type.  Assuming text\n";
	}
    }

    parsable->setContents(contents.get(), contents.length());
    return parsable;
}
