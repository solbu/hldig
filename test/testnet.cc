// $Id: testnet.cc,v 1.5 1999/10/06 12:08:13 angus Exp $
#ifdef HAVE_CONFIG_H
#include <htconfig.h>
#endif /* HAVE_CONFIG_H */

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

#include <unistd.h>

int debug = 0;
int timesvar = 1;
int persistent = 1;

URL *url;
Transport *transportConnect = NULL;
HtHTTP    *HTTPConnect = NULL;

#define DEFAULT_MAX_DOCUMENT_SIZE 40000

static void usage();
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

   while((c = getopt(ac, av, "vU:t:ng")) != -1)
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
         case 'g':
            HtHTTP::DisableHeadBeforeGet();
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
      Retrieve();
      if (debug>0)
         cout << i << ". " << endl;
   }
   
   if(debug>0)
   {
      cout << "HTTP Requests             : " << HtHTTP::GetTotRequests() << endl;
      cout << "HTTP KBytes requested     : " << (double)HtHTTP::GetTotBytes()/1024 << endl;
      cout << "HTTP Average request time : " << HtHTTP::GetAverageRequestTime()
         << " secs" << endl;
         
      cout << "HTTP Average speed        : " << HtHTTP::GetAverageSpeed()/1024
         << " KBytes/secs" << endl;
   }
         

   // Memory freeing

   if (HTTPConnect)
   {
      /// HERE DUMPS THE CORE
     delete HTTPConnect;
   }
   
   if (url) delete url;

   return 0;

}


void usage()
{
	cout << "usage: testnet [-v] [-n] [-g] [-U URL] [-t times]" << endl;
	cout << "Ht://Dig " << VERSION << endl << endl;

	cout << "Options:" << endl;

	cout << "\t-v\tVerbose mode" << endl << endl;

	cout << "\t-U URL" << endl;
	cout << "\t\tURL to be retrieved" << endl << endl;

	cout << "\t-t times" << endl;
	cout << "\t\tTimes to retrieve it" << endl << endl;

	cout << "\t-n\tNormal connection (disable persistent)" << endl << endl;

	cout << "\t-g\tOnly GET requests instead of HEAD+GET" << endl << endl;

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


Transport::DocStatus Retrieve()
{
    // Right now we just handle http:// service
    // Soon this will include file://
    // as well as an ExternalTransport system
    // eventually maybe ftp:// and a few others

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

	if (!HTTPConnect)
        {

            if (debug>1)
            cout << "Creating an HtHTTP object" << endl;
      
            HTTPConnect = new HtHTTP();

            if (!HTTPConnect)
               reportError(strerror(errno));
        }

        if (HTTPConnect)
        {
            // Here we must set only thing for a HTTP request
            
	    HTTPConnect->SetRequestURL(*url);

            HTTPConnect->SetRequestMaxDocumentSize(DEFAULT_MAX_DOCUMENT_SIZE);

            // Set the referer

            // We may issue a config paramater to enable/disable them
            if (!persistent) HTTPConnect->DisablePersistentConnection();
                      
            // HTTPConnect->SetRequestMethod(HtHTTP::Method_GET);
            if (debug > 2)
            {
               cout << "Making HTTP request on " << url->get();
               cout << endl;
            }
        }
        
	transportConnect = HTTPConnect;
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



int Parser(char *)
{
   return false;
}
