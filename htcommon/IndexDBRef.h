//
// IndexDBRef.h
//
// IndexDBRef: Reference to an indexed document. Keeps track of all
//              information stored on the document, either by the dig 
//              or temporary search information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: IndexDBRef.h,v 1.1.2.2 2005/11/28 18:37:43 aarnone Exp $
//

#ifndef _IndexDBRef_h_
#define _IndexDBRef_h_

#include "htString.h"
#include "List.h"

#include <time.h>


enum indexDBRefState
{
    indexDBRef_normal,
    indexDBRef_not_found,
    indexDBRef_noindex,
    indexDBRef_obsolete
};

class IndexDBRef : public Object
{
    public:
    //
    // Construction/Destruction
    //
    IndexDBRef();
    ~IndexDBRef();


    //
    // Get functions
    //
    String          DocURL()                {return URL;}
    time_t          DocTime()               {return time;}
//    List            *Descriptions()         {return &descriptions;}
    indexDBRefState DocState()              {return state;}
    int             DocSig()                {return sig;} 
    int             DocHopCount()           {return hopCount;}
    int             DocBacklinks()          {return backlinks;}

    //
    // Set functions
    // 
    void        DocURL(const char *u)       {URL = u;}
    void        DocTime(time_t t)           {time = t;}
//    void        Descriptions(List &l)       {descriptions = l;}
    void        DocState(indexDBRefState s) {state = s;}
    void        DocSig(int i)               {sig = i;}
    void        DocHopCount(int i)          {hopCount = i;}
    void        DocBacklinks(int i)         {backlinks = i;} 

//    void        AddDescription(const char *d, HtWordList &words);
 

    //
    // An IndexDBRef can read itself from a character string
    // and convert itself into a character string
    //
    void        Serialize(String &s);
    void        Deserialize(String &s);

    //
    // Reset Everything
    // 
    void        Clear();



    protected:
    //
    // These values will be stored when serializing
    //

    // This is the URL of the document.
    String      URL;
    // This is the time specified in the document's header
    // Usually that's the last modified time, for servers that return it.
    time_t      time;
    // This is a list of Strings, the text of links pointing to this document.
    // (e.g. <a href="docURL">description</a>
//    List        descriptions;
    // This is a signature of the document. (e.g. md5sum, checksum...)
    // This is currently unused.
    long int    sig;
    // This is the state of the document--modified, normal, etc.
    indexDBRefState  state;
    // This is a count of the number of hops from start_urls to here.
    int         hopCount;
    // This is a count of the links to the document (incoming links).
    int         backlinks;


};

#endif


