//
// Transport.h
//
// Transport: A virtual transport interface class for accessing
//            remote documents. Used to grab URLs based on the 
//            scheme (e.g. http://, ftp://...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Transport.h,v 1.1.2.1 2006/09/25 23:51:00 aarnone Exp $
//
//

#ifndef _Transport_H
#define _Transport_H

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Object.h"
#include "HtDateTime.h"
#include "htString.h"
#include "URL.h"
#include "Connection.h"
#include "HtDebug.h"

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

// Declare in advance
class Transport;

// But first, something completely different. Here's the response class
class Transport_Response : public Object
{
    friend class Transport;    // declaring friendship

    public:

    ///////
    //    Construction / Destruction
    ///////

    Transport_Response();
    virtual ~Transport_Response();

    // Reset the information stored
    virtual void Reset();  // This function must be defined

    // Get the contents
    virtual const String &GetContents() const { return _contents; }

    // Get the modification time object pointer
    virtual HtDateTime *GetModificationTime() const { return _modification_time; }

    // Get the access time object pointer
    virtual HtDateTime *GetAccessTime() const { return _access_time; }

    // Get the Content type
    virtual const String &GetContentType() const { return _content_type; }

    // Get the Content length
    virtual int GetContentLength() const { return _content_length; }

    // Get the Document length (really stored)
    virtual int GetDocumentLength() const { return _document_length; }

    // Get the Status Code
    virtual int GetStatusCode() const { return _status_code; }

    // Get the Status Code reason phrase
    virtual const String &GetReasonPhrase() { return _reason_phrase; }

    // Get the location (redirect)
    virtual const String &GetLocation() { return _location; }


    protected:

    // Body of the response message

    String	   _contents;	   	    // Contents of the document

    HtDateTime  *_modification_time; // Modification time returned by the server
    HtDateTime  *_access_time;       // Access time returned by the server

    String	     _content_type; 	  // Content-type returned by the server
    int   	     _content_length;     // Content-length returned by the server
    int   	     _document_length;    // Length really stored

    int   	   _status_code;  	  // return Status code
    String	   _reason_phrase;	  // status code reason phrase

    String    _location;	          // Location (in case of redirect) 

};


///////
//    Transport class declaration
///////

class Transport : public Object
{
    public:

        ///////
        //    Construction / Destruction
        ///////

        Transport(Connection* _connection = 0);
        virtual ~Transport();

        ///////
        //    Enumeration of possible return status of a resource retrieving
        ///////

        enum DocStatus
        {
            Document_ok,
            Document_not_changed,
            Document_not_found,
            Document_not_parsable,
            Document_redirect,
            Document_not_authorized,
            Document_no_connection,
            Document_connection_down,
            Document_no_header,
            Document_no_host,
            Document_no_port,
            Document_not_local,
            Document_not_recognized_service,    // Transport service not recognized
            Document_other_error                // General error (memory)
        };


        ///////
        //    Connects to an host and a port
        //    Overloaded methods provided in order to take
        //     the info from a URL obj or ptr
        ///////

        // Set Connection parameters
        virtual void SetConnection (const String &host, int port);

        // from a URL pointer
        virtual void SetConnection (URL *u)
        { SetConnection (u->host(), u->port()); }

        // from a URL object
        virtual void SetConnection (URL &u)
        { SetConnection (&u); }



        // Make the request
        virtual DocStatus Request() = 0; // different in derived classes



        // Get the date time information about the request
        const HtDateTime *GetStartTime() const { return &_start_time; }
        const HtDateTime *GetEndTime() const { return &_end_time; }

        // Set and get the connection time out value
        void SetTimeOut ( int t ) { _timeout=t; }
        int GetTimeOut () { return _timeout; }

        // Set and get the connection retry number
        void SetRetry ( int r ) { _retries=r; }
        int GetRetry () { return _retries; }

        // Set and get the wait time after a failed connection
        void SetWaitTime ( unsigned int t ) { _wait_time = t; }
        unsigned int GetWaitTime () { return _wait_time; }

        // Get the Connection Host
        const String &GetHost() { return _host; }

        // Get the Connection IP Address
        const String &GetHostIPAddress() { return _ip_address; }

        // Get the Connection Port
        int GetPort() { return _port; }

        // Set and get the credentials
        // Likely to vary based on transport protocol
        virtual void SetCredentials (const String& s) { _credentials = s;}
        virtual String GetCredentials () { return _credentials;}

        // Proxy settings
        virtual void SetProxy(int aUse) { _useproxy=aUse; }

        // Proxy credentials
        virtual void SetProxyCredentials (const String& s) { _proxy_credentials = s;}
        virtual String GetProxyCredentials () { return _proxy_credentials;}

        // Set the modification date and time for If-Modified-Since   
        void SetRequestModificationTime (HtDateTime *p) { _modification_time=p; }
        void SetRequestModificationTime (HtDateTime &p)
        { SetRequestModificationTime (&p) ;}

        // Get the modification date time
        HtDateTime *GetRequestModificationTime () { return _modification_time; }

        // Get and set the max document size to be retrieved
        void SetRequestMaxDocumentSize (int s) { _max_document_size=s; }
        int GetRequestMaxDocumentSize() const { return _max_document_size; }

        virtual Transport_Response *GetResponse() = 0;

        virtual DocStatus GetDocumentStatus() = 0; 

        ///////
        //    Querying the status of the connection
        ///////

        // Are we still connected?
        // This is the only part regarding
        // a connection that's got a public access

        virtual bool isConnected(){ return _connection?_connection->IsConnected():0; }

        // Set the default parser string for the content-type
        static void SetDefaultParserContentType (const String &ct)
        { _default_parser_content_type = ct; }

        // Get statistics info
        static int GetTotOpen () { return _tot_open; }   
        static int GetTotClose () { return _tot_close; }   
        static int GetTotServerChanges () { return _tot_changes; }   


    protected:

        ///////
        //    Services about a Transport layer connection
        //    They're invisible from outside
        ///////

        // Open the connection

        virtual int OpenConnection();

        // Assign the host and the port for the connection

        int AssignConnectionServer();
        int AssignConnectionPort();   

        // Connect to the specified host and port
        int Connect();

        // Write a message
        inline int ConnectionWrite(char *cmd)
        { return _connection?_connection->Write(cmd):0; }


        // Assign the timeout to the connection (returns the old value)

        inline int AssignConnectionTimeOut()
        { return _connection?_connection->Timeout(_timeout):0; }

        // Assign the retry number to the connection (returns the old value)

        inline int AssignConnectionRetries()
        { return _connection?_connection->Retries(_retries):0; }

        // Assign the wait time (after a failure) to the connection

        inline int AssignConnectionWaitTime()
        { return _connection?_connection->WaitTime(_wait_time):0; }

        // Flush the connection
        void FlushConnection();

        // Close the connection

        int CloseConnection();

        // Reset Stats
        static void ResetStatistics ()
        { _tot_open=0; _tot_close=0; _tot_changes=0;}   

        // Show stats
        static ostream &ShowStatistics (ostream &out);

        // Methods for manipulating date strings -- useful for subclasses

        enum DateFormat
        {
            DateFormat_RFC1123,
            DateFormat_RFC850,
            DateFormat_AscTime,
            DateFormat_NotRecognized
        };

        //    Create a new HtDateTime object
        HtDateTime *NewDate(const char *);

        //    Recognize Date Format
        DateFormat RecognizeDateFormat (const char *);

    protected:

        Connection	*_connection;	       // Connection object

        String       _host;                 // TCP Connection host
        String       _ip_address;           // TCP Connection host (IP Address)
        int          _port;                 // TCP Connection port

        int		_timeout;              // Connection timeout
        int		_retries;              // Connection retry limit
        unsigned int _wait_time;            // Connection wait time (if failed)

        HtDateTime	*_modification_time;   // Stored modification time if avail.
        int		_max_document_size;    // Max document size to retrieve

        String	_credentials;	       // Credentials for this connection

        int		_useproxy;	    // if true, GET should include full url,
        // not path only
        String	_proxy_credentials; // Credentials for this proxy connection

        HtDateTime  _start_time;         // Start time of the request
        HtDateTime  _end_time;           // end time of the request


        ///////
        //    Default parser content-type
        //    This string is matched in order to determine
        //    what content type can be considered parsed
        //    directly by the internal indexer (not by using
        //    any external parser)
        ///////

        static String  _default_parser_content_type;


        // Statistics about requests
        static int  _tot_open;  	 // Number of connections opened
        static int  _tot_close;  	 // Number of connections closed
        static int  _tot_changes;  	 // Number of server changes

        // Use the HTTP Basic Digest Access Authentication method to write a String
        // to be used for credentials (both HTTP and HTTP PROXY authentication)
        static void SetHTTPBasicAccessAuthorizationString(String &dest, const String& s);

        ///////
        //    Debug outlogger
        ///////
        HtDebug * debug;

};

#endif

