// $Id: testnet.cc,v 1.2 1999/09/28 17:36:55 loic Exp $

#include "Transport.h"
#include "HtHTTP.h"
#include "HtDateTime.h"
#include <URL.h>
#include <iostream.h>
#include <errno.h>
#include <string.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int debug = 0;
int timesvar = 1;
int persistent = 1;

URL *url;
Transport *transportConnect = NULL;

#define DEFAULT_MAX_DOCUMENT_SIZE 40000

static void usage();
void CreateHTTP(Transport **);
void reportError(char *msg);
Transport::DocStatus Retrieve();
int Parser(char *ct);



int main(int ac, char **av)
{
///////
   //	Local variables
///////

   // Url to be retrieved
   String URL_To_Retrieve="";
   
   // Character for get_opt function
   int c;
   

///////
   //	Retrieving options from command line with getopt
///////

   while((c = getopt(ac, av, "vU:t:n")) != -1)
   {
      switch (c)
      {
         case 'v':
            debug++;
            break;
         case 'U':
            URL_To_Retrieve=optarg;
            break;
         case 't':
            timesvar=atoi(optarg);
            break;
         case 'n':
            persistent = 0;
            break;
         case '?':
            usage();
      }
   }

   if (URL_To_Retrieve.length() == 0) usage();

   // Create the new URL
   
   url = new URL (URL_To_Retrieve);
   
   if (!url) reportError(strerror(errno));


   if (debug>0)
      cout << "Testing the net for " << url->get() << " - " << url->service() << endl;

   Transport::SetDebugLevel(debug);
   HtHTTP::SetParsingController(Parser);
   int i;
   
   for (i=0; i < timesvar; i++)
   {
      if (debug>0)
         cout << i << ". " << endl;
      Retrieve();
   }
   

   // Memory freeing
   
   if (transportConnect) delete transportConnect;
   
   if (url) delete url;

   if(debug>0)
      cout << "Finished. Average: " << HtHTTP::GetAverageRequestTime() << endl;
      
   return 0;

}


void usage()
{
	cout << "usage: testnet [-v] [-U URL]" << endl;
	cout << "Ht://Dig " << VERSION << endl << endl;

	cout << "Options:" << endl;

	cout << "\t-v\tVerbose mode" << endl << endl;

	cout << "\t-U URL" << endl;
	cout << "\t\tURL to be retrieved" << endl << endl;

	cout << "\t-t times" << endl;
	cout << "\t\tTimes to retrieve it" << endl << endl;

	cout << "\t-n\tNormal connection (disable persistent)" << endl << endl;

	exit(1);
}


//
// Report an error and die
//
void reportError(char *msg)
{
    cout << "testnet: " << msg << "\n\n";
    exit(1);
}


void CreateHTTP(Transport **pTransport)
{
   if (debug>1)
      cout << "Creating an HtHTTP object" << endl;
      
   *pTransport = new HtHTTP();
   
   if (!*pTransport) reportError(strerror(errno));

   (*pTransport)->SetRequestMaxDocumentSize(DEFAULT_MAX_DOCUMENT_SIZE);
   
}






Transport::DocStatus Retrieve()
{
    // Right now we just handle http:// service
    // Soon this will include file://
    // as well as an ExternalTransport system
    // eventually maybe ftp:// and a few others

    static HtHTTP		*http = 0;

    Transport::DocStatus	status;
    Transport_Response		*response = 0;
    HtDateTime 			*ptrdatetime = 0;
    HtDateTime 			modtime;

    String contents;
    String contentType;
    int contentLength;

    transportConnect = 0;

    if (mystrncasecmp(url->service(), "http", 4) == 0)
    {
	if (!http)
	  CreateHTTP((Transport **)&http);

        if (http)
        {
            // Here we must set only thing for a HTTP request
            
	    http->SetRequestURL(*url);

            // Set the referer

            // We may issue a config paramater to enable/disable them
            if (!persistent) http->DisablePersistentConnection();
                      
            // http->SetRequestMethod(HtHTTP::Method_GET);
            if (debug > 2)
            {
               cout << "Making HTTP request on " << url->get();

//               if (useproxy)
//                 cout << " via proxy (" << proxy->host() << ":" << proxy->port() << ")";

               cout << endl;
            }
        }
        
	transportConnect = http;
    }
    else
    {
	if (debug)
	{
	    cout << '"' << url->service() <<
		"\" not a recognized transport service. Ignoring\n";
	}
    }

    // Is a transport object pointer available?

    if (transportConnect)
      {

         transportConnect->SetConnection(url);

	// Make the request
	// Here is the main operation ... Let's make the request !!!
	status = transportConnect->Request();

	// Let's get out the info we need
	response = transportConnect->GetResponse();

	if (response)
	{
	   // We got the response

	   contents = response->GetContents();
	   contentType = response->GetContentType();
	   contentLength = response->GetContentLength();
	   ptrdatetime = response->GetModificationTime();

           if (ptrdatetime)
           {
		// We got the modification date/time
		modtime = *ptrdatetime;
           }
           // How to manage it when there's no modification date/time?

           if (debug > 5)
           {
              cout << "Contents:\n" << contents << endl;
              cout << "Content Type: " << contentType << endl;
              cout << "Content Lenght: " << contentLength << endl;
              cout << "Modification Time: " << modtime.GetISO8601() << endl;
           }
	}

	return status;

      }
    else
      return Transport::Document_not_found;

}



int Parser(char *ct)
{
   return false;
}
