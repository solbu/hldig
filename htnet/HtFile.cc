//
// HtFile.cc
//
// HtFile: Interface classes for retriving local documents
//
// Including:
// 	 -  Generic class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtFile.cc,v 1.1.2.6 2000/09/08 04:26:24 ghutchis Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "lib.h"
#include "Transport.h"
#include "HtFile.h"
#include "Dictionary.h"
#include "StringList.h"
#include "defaults.h" // for config

#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <iostream.h>
#include <stdio.h> // for sscanf
#include <sys/stat.h>
#include <unistd.h>
#include <fstream.h>
#include <dirent.h> // for scandir


///////
   //    HtFile_Response class
///////


// Construction

HtFile_Response::HtFile_Response()
{
}


// Destruction

HtFile_Response::~HtFile_Response()
{
}

///////
   //    HtFile generic class
   //
   //    
///////


// Construction

HtFile::HtFile()
{
   _modification_time=NULL;
}

// Destruction

HtFile::~HtFile()
{
   // It's empty
}


///////
   //    Manages the requesting process
///////

HtFile::DocStatus HtFile::Request()
{
   static Dictionary *mime_map = 0;

   if (!mime_map)
     {
       ifstream in(config["mime_types"].get());
       if (in)
         {
           mime_map = new Dictionary();
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

       struct dirent **namelist;
       int n;
       String filename;

       n = scandir(_url.path(), &namelist, 0, alphasort);
       if (n > 0)
	 {
	   for (int i = 0; i < n; i++)
	     {
	       filename = _url.path();
	       filename << namelist[i]->d_name;

	       cout << " file name " << filename << endl;

	       if ( namelist[i]->d_name[0] != '.' 
		    && stat(filename.get(), &stat_buf) == 0 )
		 {
		   if (S_ISDIR(stat_buf.st_mode))
		     _response._contents << "<link href=\"file://" << _url.path()
					 << "/" << namelist[i]->d_name << "/\">\n";
		   else
		     _response._contents << "<link href=\"file://" << _url.path()
					 << "/" << namelist[i]->d_name << "\">\n";  
		 }
	     }
	   delete [] namelist;
	 }
       _response._contents << "</head><body></body></html>\n";

       if (debug > 3)
	 cout << " Directory listing: " << endl << _response._contents << endl;

       _response._content_length = stat_buf.st_size;
       _response._document_length = _response._contents.length();
       _response._modification_time = new HtDateTime(stat_buf.st_mtime);
       _response._status_code = 0;
       return Transport::Document_ok;
     }

   if (_modification_time && *_modification_time >= HtDateTime(stat_buf.st_mtime))
     return Transport::Document_not_changed;

   char *ext = strrchr(_url.path(), '.');
   if (ext == NULL)
     return Transport::Document_not_local;

   if (mime_map)
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

   ifstream in((const char *)_url.path());
   if (!in)
     return Document_not_found;

   String tmp;
   while (in >> tmp)
     {
       if (_response._contents.length()+tmp.length() > _max_document_size)
         tmp.chop(_response._contents.length()+tmp.length()
                    - _max_document_size);
       _response._contents.append(tmp);
       if (_response._contents.length() >= _max_document_size)
         break;
     }

   _response._content_length = stat_buf.st_size;
   _response._document_length = _response._contents.length();
   _response._status_code = 0;

   if (debug > 2)
     cout << "Read a total of " << _response._document_length << " bytes\n";
   return Transport::Document_ok;
}

HtFile::DocStatus HtFile::GetDocumentStatus()
{ 
   // Let's give a look at the return status code
   if (_response._status_code == -1)
      return Transport::Document_not_found;
   return Transport::Document_ok;
}

