//
// Document.cc
//
// Implementation of Document
//
// $Log: Document.cc,v $
// Revision 1.22  1998/11/09 19:35:09  ghutchis
//
// Changed reset to keep proxy settings--fixes bug noted by Didier Gautheron
// <dgautheron@magic.fr>.
//
// Revision 1.21  1998/11/02 20:30:35  ghutchis
//
// Remove const from *ext to fix compiler warning.
//
// Revision 1.20  1998/11/01 00:00:40  ghutchis
//
// Replaced system calls with htlib/my* functions.
//
// Revision 1.19  1998/10/26 20:43:31  ghutchis
//
// Fixed bug introduced by Oct 18 change. Authorization will not be cleared.
//
// Revision 1.18  1998/10/18 21:22:16  ghutchis
//
// Revised connection timeout methods.
//
// Revision 1.17  1998/10/12 02:04:00  ghutchis
//
// Updated Makefiles and configure variables.
//
// Revision 1.15  1998/09/08 03:29:09  ghutchis
//
// Clean up for 3.1.0b1.
//
// Revision 1.14  1998/09/06 03:22:37  ghutchis
//
// Bug fixes
//
// Revision 1.13  1998/08/03 16:50:31  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.12  1998/07/23 16:18:52  ghutchis
//
// Added files (and patch) from Sylvain Wallez for PDF
// parsing. Incorporates fix for non-Adobe PDFs.
//
// Revision 1.11  1998/07/09 09:38:57  ghutchis
//
//
// Added support for local file digging using patches by Pasi. Patches
// include support for local user (~username) digging.
//
// Revision 1.10  1998/01/05 05:42:57  turtle
// Changed tm from pointer to real structure
//
// Revision 1.9  1998/01/05 00:59:40  turtle
// *  Alarm was not cancelled if readHeader returned anything but OK
// *  Use our own timegm() replacement if necessary
//
// Revision 1.8  1997/12/16 15:57:22  turtle
// Added little patch by Tobias Oetiker <oetiker@ee.ethz.ch> that should
// fix problems with timeouts.
//
// Revision 1.7  1997/07/07 22:15:45  turtle
// Removed old getdate() code that replaced '-' with ' '.
//
// Revision 1.6  1997/07/07 21:22:14  turtle
// Added better date parsing.  Now also supports the old RFC 850 format
//
// Revision 1.5  1997/07/03 17:44:37  turtle
// Added support for virtual hosts
//
// Revision 1.4  1997/06/14 18:52:42  turtle
// Made redirect detection code more general
//
// Revision 1.3  1997/04/20 15:25:17  turtle
// Added include for ctype.h
//
// Revision 1.2  1997/02/10 17:32:37  turtle
// Applied AIX specific patches supplied by Lars-Owe Ivarsson
// <lars-owe.ivarsson@its.uu.se>
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Document.cc,v 1.22 1998/11/09 19:35:09 ghutchis Exp $";
#endif

#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>
#include "Document.h"
#include "Connection.h"
#include "htdig.h"
#include "HTML.h"
#include "Plaintext.h"
#include "Postscript.h"
#include "ExternalParser.h"
#include "PDF.h"

#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif

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
    contents = 0;

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
    if (!url)
      delete url;
    if (!proxy)
      delete proxy;
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
    contentType = 0;
    if (!url)
      delete url;
    url = 0;
    referer = 0;
    if(config.Boolean("modification_time_is_now"))
       modtime = time(NULL);
    else
       modtime = 0;

    contents = 0;
    document_length = 0;
    redirected_to = 0;

    // Don't reset the authorization since it's a pain to set up again.
    //    authorization = 0;
    // Don't reset the proxy since it's a pain to set up too.
    //    if (!proxy)
    //      delete proxy;
    //    proxy = 0;
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
    if (!url)
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
    struct tm   tm;
    
    //
    // Two possible time designations:
    //      Tuesday, 01-Jul-97 16:48:02 GMT
    // or
    //      Thu, 01 May 1997 00:40:42 GMT
    //
    if (d.indexOf(',') > 3)
	mystrptime(d.get(), "%a, %d-%b-%y %T", &tm);
    else
	mystrptime(d.get(), "%a, %d %b %Y %T", &tm);

    if (tm.tm_year < 0)
	tm.tm_year += 1900;

    if (debug > 2)
    {
	cout << "Translated " << d << " to ";
	char	buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T", &tm);
	cout << buffer << " (" << tm.tm_year << ")" << endl;
    }
    time_t      ret;
#if HAVE_TIMEGM
    ret = timegm(&tm);
#else
    ret = mytimegm(&tm);
#endif
    if (debug > 2)
    {
        cout << "And converted to ";
        struct tm *tm2 = gmtime(&ret);
        char    buffer[100];
        strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T", tm2);
        cout << buffer << endl;
    }
    return ret;
}


//*****************************************************************************
// DocStatus Document::RetrieveHTTP(time_t date)
//   Attempt to retrieve the document pointed to by our internal URL
//
Document::DocStatus
Document::RetrieveHTTP(time_t date)
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
    command << "User-Agent: " << config["user_agent"] << "/" 
	    << VERSION << " (" <<	config["maintainer"] << ")\r\n";

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
    // If we are allowed to index virtual hosts, we will send the special
    // 'Host:' header that tells the server what virtual web site this
    // request is for.
    //
    if (config.Boolean("allow_virtual_hosts", 1))
    {
	command << "Host: " << url->host() << "\r\n";
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
    c.timeout(config.Value("timeout"));

    DocStatus   returnStatus = Document_ok;;
    switch (readHeader(c))
    {
	case Header_ok:
            returnStatus = Document_ok;
	    break;
	case Header_not_changed:
	    returnStatus = Document_not_changed;
            break;
	case Header_not_found:
	    returnStatus = Document_not_found;
            break;
	case Header_redirect:
	    returnStatus = Document_redirect;
            break;
	case Header_not_text:
	    returnStatus = Document_not_html;
            break;
	case Header_not_authorized:
	    returnStatus = Document_not_authorized;
            break;
    }
    if (returnStatus != Document_ok)
        return returnStatus;

    //
    // Read in the document itself
    //
    contents = 0;
    char	docBuffer[8192];
    int		bytesRead;

    while ((bytesRead = c.read(docBuffer, sizeof(docBuffer))) > 0)
    {
	if (debug > 2)
	    cout << "Read " << bytesRead << " from document\n";
	if (contents.length() + bytesRead > max_doc_size)
	    break;
	contents.append(docBuffer, bytesRead);
    }
    c.close();
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

    if(config.Boolean("modification_time_is_now"))
       modtime = time(NULL);
    else
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
		else if (status && strcmp(status, "304") == 0)
		{
		    returnStatus = Header_not_changed;
		}
		else if (status && strncmp(status, "30", 2) == 0)
		{
		    //
		    // All 3xx codes other than 304 will be considered
		    // HTTP redirects that need to look at the
		    // Location header field.
		    //
		    returnStatus = Header_redirect;
		}
		else if (status && strcmp(status, "401") == 0)
		{
		    returnStatus = Header_not_authorized;
		}
	    }
	    else if (modtime == 0 && mystrncasecmp(line, "last-modified:", 14) == 0)
	    {
		strtok(line, " \t");
		modtime = getdate(strtok(0, "\n\t"));
	    }
	    else if (mystrncasecmp(line, "content-type:", 13) == 0)
	    {
		strtok(line, " \t");
		char	*token = strtok(0, "\n\t");
				
		if (mystrncasecmp("text/", token, 5) != 0 &&
		    mystrncasecmp("application/postscript", token, 22) != 0 &&
		    mystrncasecmp("application/msword", token, 18) != 0 &&
		    mystrncasecmp("application/pdf", token, 15) != 0)
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
// DocStatus Document::RetrieveLocal(time_t date, char *filename)
//   Attempt to retrieve the document pointed to by our internal URL
//   using a local filename given. Returns Document_ok,
//   Document_not_changed or Document_not_local (in which case the
//   retriever tries it again using HTTP).
//
Document::DocStatus
Document::RetrieveLocal(time_t date, char *filename)
{
    struct stat stat_buf;
    // Check that it exists, and is a regular file. 
    if ((stat(filename, &stat_buf) == -1) || !S_ISREG(stat_buf.st_mode))
	return Document_not_local;

    modtime = stat_buf.st_mtime;
    if (modtime <= date)
        return Document_not_changed;

    // Process only HTML files (this could be changed if we read
    // the server's mime.types file).
    char *ext = strrchr(filename, '.');
    if (ext == NULL)
      	return Document_not_local;
    if ((mystrcasecmp(ext, ".html") == 0) || (mystrcasecmp(ext, ".htm") == 0))
        contentType = "text/html";
    else 
  	return Document_not_local;

    // Open it
    FILE *f = fopen(filename, "r");
    if (f == NULL)
 	return Document_not_local;

    //
    // Read in the document itself
    //
    contents = 0;
    char	docBuffer[8192];
    int		bytesRead;

    while ((bytesRead = fread(docBuffer, 1, sizeof(docBuffer), f)) > 0)
    {
	if (debug > 2)
	    cout << "Read " << bytesRead << " from document\n";
	if (contents.length() + bytesRead > max_doc_size)
	    break;
	contents.append(docBuffer, bytesRead);
    }
    fclose(f);
    document_length = contents.length();

    if (debug > 2)
	cout << "Read a total of " << document_length << " bytes\n";
    return Document_ok;
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
    static PDF			*pdf = 0;
    
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
    else if (mystrncasecmp(contentType, "application/pdf", 15) == 0)
    {
	if (!pdf)
	    pdf = new PDF();
	parsable = pdf;
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
