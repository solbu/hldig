//
// HtFTP.cc
//
// HtFTP: Interface classes for retriving documents from FTP servers
//
// Including:
// 	 -  Generic class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtFTP.cc,v 1.4 2003/06/24 19:58:07 nealr Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "lib.h"
#include "Transport.h"
#include "HtFTP.h"
#include "Dictionary.h"
#include "StringList.h"
#include "defaults.h" // for config

#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <iostream.h>
#include <stdio.h> // for sscanf
#include <sys/stat.h>

#ifndef _MSC_VER //_WIN32
#include <unistd.h>
#endif

#include <fstream.h>

#ifdef _MSC_VER //_WIN32
#include "dirent_local.h"
#else
#include <dirent.h> // for scandir
#endif


///////
   //    HtFTP_Response class
///////


// Construction

HtFTP_Response::HtFTP_Response()
{
}


// Destruction

HtFTP_Response::~HtFTP_Response()
{
}

///////
   //    HtFTP generic class
   //
   //    
///////


// Construction

HtFTP::HtFTP()
{
}

// Destruction

HtFTP::~HtFTP()
{
   // It's empty
}


///////
   //    Manages the requesting process
///////

//NB: HtFTP::DocStatus = return-value (defineret i Transport.h 
// Husk HtFTP er nedarvet fra Transport klassen

HtFTP::DocStatus HtFTP::Request()
{
   HtConfiguration* config= HtConfiguration::config();
   static Dictionary *mime_map = 0;

   // har vi brug for denne mime_map i denne klasse?
   // hvis ja, burde den ikke laves til function i HtConfiguration


   if (!mime_map) // generate mime_map from the current configuration 
     {
       mime_map = new Dictionary();
       ifstream in(config->Find("mime_types").get());
       if (in)
         {
           String line;
           while (in >> line)
             {
               line.chop("\n\r \t");
               int cmt;
               if ((cmt = line.indexOf('#')) >= 0)
                 line = line.sub(0, cmt);
               StringList split_line(line, "\t ");
               // Let's cache mime type to lesser the number of 
               // operator [] callings
               String mime_type = split_line[0];
               // Fill map with values.
               for (int i = 1; i < split_line.Count(); i++)
                 mime_map->Add(split_line[i], new String(mime_type));
             }
         }
     }

   // Reset the response
   _response.Reset();
   
   struct stat stat_buf;
   // Check that it exists, and is a regular file or directory
   // Should we allow FIFO's?
   if ( stat(_url.path(), &stat_buf) != 0 || 
	!(S_ISREG(stat_buf.st_mode) || S_ISDIR(stat_buf.st_mode)) )
     return Transport::Document_not_found;

   // Now handle directories with a pseudo-HTML document (and appropriate noindex)
   if ( S_ISDIR(stat_buf.st_mode) )
     {
       _response._content_type = "text/html";
       _response._contents = "<html><head><meta name=\"robots\" content=\"noindex\">\n";

       struct dirent *namelist;
       DIR *dirList;
       String filename;

       if (( dirList = opendir(_url.path()) ))
	 {
	   while (( namelist = readdir(dirList) ))
	    {
	     filename = _url.path();
	     filename << namelist->d_name;
	     
	     if ( namelist->d_name[0] != '.' 
		  && stat(filename.get(), &stat_buf) == 0 )
	       {
		 if (S_ISDIR(stat_buf.st_mode))
		   _response._contents << "<link href=\"file://" << _url.path()
				       << "/" << namelist->d_name << "/\">\n";
		 else
		   _response._contents << "<link href=\"file://" << _url.path()
				       << "/" << namelist->d_name << "\">\n";  
	       }
	    }
	   closedir(dirList);
	 }

       _response._contents << "</head><body></body></html>\n";

       if (debug > 4)
	 cout << " Directory listing: " << endl << _response._contents << endl;

       _response._content_length = stat_buf.st_size;
       _response._document_length = _response._contents.length();
       _response._modification_time = new HtDateTime(stat_buf.st_mtime);
       _response._status_code = 0;
       return Transport::Document_ok;
     }

   if (_modification_time && *_modification_time >= HtDateTime(stat_buf.st_mtime))
     return Transport::Document_not_changed;

   const char *ext = strrchr(_url.path(), '.');
   if (ext == NULL)
     return Transport::Document_not_local;

   if (mime_map && mime_map->Count())
     {
       String *mime_type = (String *)mime_map->Find(ext + 1);
       if (mime_type)
         _response._content_type = *mime_type;
       else
         return Transport::Document_not_local;
     }
   else
     {
       if ((mystrcasecmp(ext, ".html") == 0) || (mystrcasecmp(ext, ".htm") == 0))
         _response._content_type = "text/html";
       else if (mystrcasecmp(ext, ".txt") == 0)
         _response._content_type = "text/plain";
       else
         return Transport::Document_not_local;
     }

   _response._modification_time = new HtDateTime(stat_buf.st_mtime);

   FILE *f = fopen((const char *)_url.path(), "r");
   if (f == NULL)
     return Document_not_found;

   char	docBuffer[8192];
   int		bytesRead;
   while ((bytesRead = fread(docBuffer, 1, sizeof(docBuffer), f)) > 0)
     {
	if (_response._contents.length() + bytesRead > _max_document_size)
	    bytesRead = _max_document_size - _response._contents.length();
	_response._contents.append(docBuffer, bytesRead);
	if (_response._contents.length() >= _max_document_size)
	    break;
     }
   fclose(f);

   _response._content_length = stat_buf.st_size;
   _response._document_length = _response._contents.length();
   _response._status_code = 0;

   if (debug > 2)
     cout << "Read a total of " << _response._document_length << " bytes\n";
   return Transport::Document_ok;
}

HtFTP::DocStatus HtFTP::GetDocumentStatus()
{ 
   // Let's give a look at the return status code
   if (_response._status_code == -1)
      return Transport::Document_not_found;
   return Transport::Document_ok;
}

