//
// DocumentRef.h
//
// DocumentRef: Reference to an indexed document. Keeps track of all
//              information stored on the document, either by the dig 
//              or temporary search information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: DocumentRef.h,v 1.29.2.2 2005/10/07 21:36:13 aarnone Exp $
//

#ifndef _DocumentRef_h_
#define _DocumentRef_h_

#include "htString.h"
#include "List.h"
// Anthony - remove htword stuff
//#include "HtWordList.h"

#include <time.h>
#define CL_Doc std::map<std::string, std::pair<std::string, std::string> >
#define uniqueWordsSet std::set<std::string> 

enum ReferenceState
{
    Reference_normal,
    Reference_not_found,
    Reference_noindex,
    Reference_obsolete
};

class DocumentRef : public Object
{
    public:
    //
    // Construction/Destruction
    //
    DocumentRef();
    ~DocumentRef();

    //
    // A DocumentRef can read itself from a character string and
    // convert itself into a character string
    //
    void		Serialize(String &s);
    void		Deserialize(String &s);

    //
    // Access to the members
    //
    int         DocID()             {return docID;}
    char        *DocURL()           {return docURL;}
    time_t      DocTime()           {return docTime;}
    char        *DocTitle()         {return docTitle;}
    char        *DocAuthor()        {return docAuthor;}
    char        *DocHead()          {return docHead;}
    int         DocHeadIsSet()      {return docHeadIsSet;}
    char        *DocMetaDsc()       {return docMetaDsc;}
    time_t      DocAccessed()       {return docAccessed;}
    int         DocLinks()          {return docLinks;}
    int         DocBackLinks()      {return docBackLinks;}
    List        *Descriptions()     {return &descriptions;}
    int         DocSize()           {return docSize;}
    List        *DocAnchors()       {return &docAnchors;}
    double      DocScore()          {return docScore;}
    int         DocSig()            {return docSig;}
    int         DocAnchor()         {return docAnchor;}
    int         DocHopCount()       {return docHopCount;}
    char        *DocEmail()         {return docEmail;}
    char        *DocNotification()  {return docNotification;}
    char        *DocSubject()       {return docSubject;}
    ReferenceState  DocState()      {return docState;}
	
    void        DocID(int d)                {docID = d;}
    void        DocURL(const char *u)       {docURL = u;}
    void        DocTime(time_t t)           {docTime = t;}
    void        DocTitle(const char *t)     {docTitle = t;}
    void        DocAuthor(const char *a)    {docAuthor = a;}
    void        DocHead(const char *h)      {docHeadIsSet = 1; docHead = h;}
    void        DocMetaDsc(const char *md)  {docMetaDsc = md;}
    void        DocAccessed(time_t t)       {docAccessed = t;}
    void        DocLinks(int l)             {docLinks = l;}
    void        DocBackLinks(int l)         {docBackLinks = l;}
    void        Descriptions(List &l)       {descriptions = l;}
    void        DocState(ReferenceState s)  {docState = s;}
    void        DocSize(int s)              {docSize = s;}
    void        DocSig(int s)               {docSig = s;}
    void        DocAnchors(List &l)         {docAnchors = l;}
    void        DocScore(double s)          {docScore = s;}
    void        DocAnchor(int a)            {docAnchor = a;}
    void        DocHopCount(int h)          {docHopCount = h;}
    void        DocEmail(const char *e)     {docEmail = e;}
    void        DocSubject(const char *s)   {docSubject = s;}
    void        DocNotification(const char *n)  {docNotification = n;}

    void        DocState(int s);
    void        AddAnchor(const char *a);
// Anthony - not used
//    void        AddDescription(const char *d, HtWordList &words);
	
    void        Clear();                    // Reset everything - deprecated


// New CLucene functions

    void        initialize();               // Clear out everything, and set up the hash
    
    void        dumpUniqueWords();          // insert all of the unique words into the contents
    void        addUniqueWord(char* word);  // add a unique word
    void        insertField(const char* fieldName, char* fieldValue);
    void        appendField(const char* fieldName, char* fieldValue);

    protected:
    //
    // These values will be stored when serializing
    //

    // New hotness hash that contains everything
    // 
    CL_Doc indexDoc;

    // This will contain unique words (if that option is enabled)
    // before the document is put into the index, this needs to 
    // be flushed to the contents field
    // 
    uniqueWordsSet uniqueWords;

//*****************************************OLD STUFF**************
// Anthony - old DocumentRef variables
    // This is the index number of the document in the database.
    int         docID;
    // This is the URL of the document.
    String      docURL;
    // This is the time specified in the document's header
    // Usually that's the last modified time, for servers that return it.
    time_t      docTime;
    // This is the time that the last retrieval occurred.
    time_t      docAccessed;
    // This is the stored excerpt of the document, just text.
    String      docHead;
    // This indicates if the stored excerpt of the document has been set.
    int         docHeadIsSet;
    // This is the document-specified description.
    // For HTML, that's the META description tag.
    String      docMetaDsc;
    // This is the title of the document.
    String      docTitle;
    // This is the author of the document, as specified in meta information
    String      docAuthor;
    // This is a list of Strings, the text of links pointing to this document.
    // (e.g. <a href="docURL">description</a>
    List        descriptions;
    // This is the size of the original document.
    int         docSize;
    // This is a count of the links in the document (outgoing links).
    int         docLinks;
    // This is a count of the links to the document (incoming links).
    int         docBackLinks;
    // This is a list of the anchors in the document (i.e. <A NAME=...)
    List        docAnchors;
    // This is a count of the number of hops from start_urls to here.
    int         docHopCount;
    // This is a signature of the document. (e.g. md5sum, checksum...)
    // This is currently unused.
    long int    docSig;
    // This is the state of the document--modified, normal, etc.
    ReferenceState  docState;


    //
    // The following values are for the email notification of expiration
    //
    // This is the email destination for htnotify.
    String		docEmail;
    // This is the date that htnotify should use as comparison.
    String		docNotification;
    // This is the subject of the email sent out by htnotify.
    String		docSubject;

    
    //
    // This is used for searching and is not stored in the database
    //
    // This is the current score of this document.
    double			docScore;
    // This is the nearest anchor for the search word.
    int			docAnchor;
};

#endif


