// $Id: testnet.cc,v 1.10 2002/02/01 22:49:37 ghutchis Exp $
#ifdef HAVE_CONFIG_H
#include <htconfig.h>
#endif /* HAVE_CONFIG_H */

#include "Transport.h"
#include "HtHTTP.h"
#include "HtHTTPBasic.h"
#include "HtDateTime.h"
#include <URL.h>
#include <iostream.h>
#include <iomanip.h>
#include <errno.h>
#include <string.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <unistd.h>

#define DEFAULT_MAX_DOCUMENT_SIZE 40000

int debug = 0;
int timesvar = 1;
int persistent = 1;
int timeout = 10;
int head_before_get = 1;
int max_doc = DEFAULT_MAX_DOCUMENT_SIZE;
int retries = 1;
int waittime = 5;


URL *url;
Transport *transportConnect = NULL;
HtHTTP    *HTTPConnect = NULL;


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
   
   // Transport return Status 
   Transport::DocStatus _return_status;

   // Variable storing the number of timed out requests
   int _timed_out = 0;

   // Flag variable for errors
   int _errors = 0;

///////
   //	Retrieving options from command line with getopt
///////

   while((c = getopt(ac, av, "vU:T:t:ngm:r:w:")) != -1)
   {
      switch (c)
      {
         case 'v':
            debug++;
            break;
         case 'U':
            URL_To_Retrieve=optarg;
            break;
         case 'T':
            timesvar=atoi(optarg);
            break;
         case 't':
            timeout=atoi(optarg);
            break;
         case 'r':
            retries=atoi(optarg);
            break;
         case 'w':
            waittime=atoi(optarg);
            break;
         case 'm':
            max_doc=atoi(optarg);
            break;
         case 'n':
            persistent = 0;
            break;
         case 'g':
            head_before_get = 0;
            HtHTTP::DisableHeadBeforeGet();
            break;
         case '?':
            usage();
      }
   }

   if (URL_To_Retrieve.length() == 0) usage();

   if (!persistent)
      head_before_get=0;   // No HEAD before GET if no persistent connections
   
   // Create the new URL
   
   url = new URL ((char*)URL_To_Retrieve);
   
   if (!url) reportError(strerror(errno));


   if (debug>0)
   {
      cout << "Testing the net for " << url->get() << endl;
      cout << "Host: " << url->host() << " - Port: " << url->port()
         << " - Service: " << url->service() << endl;
      cout << endl;
   }

   Transport::SetDebugLevel(debug);
   HtHTTP::SetParsingController(Parser);
   int i;

   HtDateTime StartTime;
      
   for (i=0; i < timesvar; i++)
   {
      if (debug>0)
         cout << setw(5) << i+1 << "/" << timesvar;

      _return_status = Retrieve();
      
      if (debug>0)
      {
         cout << " | Start time: " << transportConnect->GetStartTime()->GetISO8601();
            
         cout << " | End time: " << transportConnect->GetEndTime()->GetISO8601()
            << " | ";
      }

         
      switch(_return_status)
      {
         case Transport::Document_ok:
            if(debug>0) cout << "OK ("
               << transportConnect->GetResponse()->GetStatusCode() << ")";
            break;
         case Transport::Document_not_changed:
            if(debug>0) cout << "Not changed ("
               << transportConnect->GetResponse()->GetStatusCode() << ")";
            break;
         case Transport::Document_not_found:
            if(debug>0) cout << "Not found ("
               << transportConnect->GetResponse()->GetStatusCode() << ")";
            break;
         case Transport::Document_not_parsable:
            if(debug>0) cout << "Not parsable ("
               << transportConnect->GetResponse()->GetContentType() << ")";
            break;
         case Transport::Document_redirect:
            if(debug>0) cout << "Redirected ("
               << transportConnect->GetResponse()->GetStatusCode() << ")";
            break;
         case Transport::Document_not_authorized:
            if(debug>0) cout << "Not authorized";
            break;
         case Transport::Document_no_connection:
            if(debug>0) cout << "No Connection";
            break;
         case Transport::Document_connection_down:
            if(debug>0) cout << "Connection down";
            _timed_out++;
            break;
         case Transport::Document_no_header:
            if(debug>0) cout << "No header";
            break;
         case Transport::Document_no_host:
            if(debug>0) cout << "No host";
            break;
         case Transport::Document_no_port:
            if(debug>0) cout << "No port";
            break;
         case Transport::Document_not_local:
            if(debug>0) cout << "Not local";
            break;
         case Transport::Document_not_recognized_service:
            if(debug>0) cout << "Service not recognized";
            break;
         case Transport::Document_other_error:
            if(debug>0) cout << "Other error";
            _errors++;
            break;
      }

      
      if (debug>0)
         cout << endl;
   }

   HtDateTime EndTime;

   // Memory freeing

   if (HTTPConnect)
     delete HTTPConnect;
   
   if (url) delete url;

   // Show statistics
   
   if(debug>0)
   {
      cout << endl;
      cout << "HTTP Info" << endl;
      cout << "=========" << endl;

      if (persistent)
      {
         cout << " Persistent connections    : On" << endl;
         if (head_before_get)
            cout << " HTTP/1.1 HEAD before GET  : On" << endl;
         else
            cout << " HTTP/1.1 HEAD before GET  : Off" << endl;
      }
      else
         cout << " Persistent connections : Off" << endl;

      
      cout << " Timeout value             : " << timeout << endl;

      cout << " Retries for timeout       : " << retries << endl;
      
      cout << " Sleep after timeout       : " << waittime << endl;
      
      cout << " Document requests         : " << timesvar << endl;

      HtHTTP::ShowStatistics(cout);

      cout << " Timed out                 : " << _timed_out << endl;
      cout << " Unknown errors            : " << _errors << endl;
      cout << " Elapsed time              : approximately "
         << HtDateTime::GetDiff(EndTime, StartTime) << " secs" << endl;

   }

   // Return values
   
   if (_errors) return -1;
   
   if (_timed_out) return 1;
   
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

	cout << "\t-T times" << endl;
	cout << "\t\tTimes to retrieve it" << endl << endl;

	cout << "\t-t timeout" << endl;
	cout << "\t\tTimeout value" << endl << endl;

	cout << "\t-r retries" << endl;
	cout << "\t\tNumber of retries after a timeout" << endl << endl;

	cout << "\t-w wait time" << endl;
	cout << "\t\tWait time value after a timeout" << endl << endl;

	cout << "\t-m maxdocsize" << endl;
	cout << "\t\tMax Document size to be retrieved" << endl << endl;

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
      
            HTTPConnect = new HtHTTPBasic();

            if (!HTTPConnect)
               reportError(strerror(errno));
        }

        if (HTTPConnect)
        {
            // Here we must set only thing for a HTTP request
            
	    HTTPConnect->SetRequestURL(*url);

            // Let's disable the cookies for this test
	    HTTPConnect->DisableCookies();

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

        transportConnect->SetRequestMaxDocumentSize(max_doc);
        transportConnect->SetTimeOut(timeout);
        transportConnect->SetRetry(retries);
        transportConnect->SetWaitTime(waittime);

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
