//--------------------------------------------------------------------
//
// BasicDocument.h
//
// 2/6/2002 created for libhtdig to simplify & mimic Document.cc
//
// Neal Richter nealr@rightnow.com
//
//
// BasicDocument: This class holds everything there is to know about a document.
//           The actual contents of the document may or may not be present at
//           all times for memory conservation reasons.
//
//           This is a basic extensable container for plain text holding documents.
//
//           Uses any Parser with parse method handling this class.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: BasicDocument.h,v 1.4 2004/05/28 13:15:28 lha Exp $
//
//--------------------------------------------------------------------




#ifndef _BasicDocument_h_
#define _BasicDocument_h_

#include "htString.h"
#include "Parsable.h"
#include "Object.h"
#include "StringList.h"
#include "HtDateTime.h"


class TextCollector;


class BasicDocument:public Object
{
   public:
    //
    // Construction/Destruction
    //
    BasicDocument(char *location = 0, int max_size = 0);
    ~BasicDocument();

    //
    // Interface to the document.
    //
    void  Reset();
    int   Length();

    //int                StoredLength()   {return contents.length();}

    char *Title()                          {return title;}
    void  Title(char *t)                   {title = t; document_length = -1;}
    void  Title(const String & t)          {title = t; document_length = -1;}
    int   TitleLength()                    {return title.length();}

    char *MetaContent()                    {return metacontent;}
    void  MetaContent(char *m)             {metacontent = m; document_length = -1;}
    void  MetaContent(const String & m)    {metacontent = m; document_length = -1;}
    int   MetaContentLength()              {return metacontent.length();}

    char *Contents()                       {return contents;}
    void  Contents(char *s)                {contents = s; document_length = -1;}
    void  Contents(const String & s)       {contents = s; document_length = -1;}
    int   ContentsLength()                 {return contents.length();}

    char *Location()                       {return location;}
    void  Location(char *l)                {location = l; document_length = -1;}
    void  Location(const String & l)       {location = l; document_length = -1;}
    int  LocationLength()                  {return location.length();}

    char *DocumentID()                     {return id;}
    void  DocumentID(char *ida)            {id = ida; document_length = -1;}
    void  DocumentID(const String & ida)   {id = ida; document_length = -1;}
    int   DocumentIDLength()               {return id.length();}

    char *ContentType()                    {return contentType;}
    void  ContentType(char *ct)            {contentType = ct;}
    void  ContentType(const String & ct)   {contentType = ct;}
    
    time_t ModTime()                       {return modtime.GetTime_t();}
    void   ModTime(time_t t)               {modtime = t;}

    //
    // Return an appropriate parsable object for the document type.
    //
    Parsable *getParsable();

    int     internalParser(TextCollector & textcollector);
    int     SelfParseable();

   private:

    String          id;
    String          location;
    String          title;
    String          metacontent;
    String          contents;

    String          contentType;

    HtDateTime      modtime;

    int             document_length;
    
    //int max_doc_size;

};

#endif
