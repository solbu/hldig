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
// $Id: HtFile.cc,v 1.2 2000/02/19 05:29:04 ghutchis Exp $ 
//

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
   // Check that it exists, and is a regular file.
   // Should we allow FIFO's?
   if (stat(_url.path(), &stat_buf) != 0 || !S_ISREG(stat_buf.st_mode))
     return Document_not_found;
   if (_modification_time && *_modification_time >= HtDateTime(stat_buf.st_mtime))
     return Document_not_changed;

   char *ext = strrchr(_url.path(), '.');
   if (ext == NULL)
     return Document_not_local;

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

   ifstream in(_url.path());
   if (!in)
     return Document_not_found;

   String tmp;
   while (in >> tmp)
     _response._contents.append(tmp);

   _response._content_length = _response._contents.length();
   _response._document_length = _response._contents.length();
   _response._status_code = 0;

   if (debug > 2)
     cout << "Read a total of " << _response._document_length << " bytes\n";
   return Document_ok;
}

HtFile::DocStatus HtFile::GetDocumentStatus()
{ 
   // Let's give a look at the return status code
   if (_response._status_code == -1)
      return Document_not_found;
   return Document_ok;
}

