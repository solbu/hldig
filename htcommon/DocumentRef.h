//
// DocumentRef.h
//
// $Id: DocumentRef.h,v 1.14.2.1 1999/03/23 01:27:27 grdetil Exp $
//
//
#ifndef _DocumentRef_h_
#define _DocumentRef_h_

#include "htString.h"
#include "List.h"
#include <time.h>

enum ReferenceState
{
    Reference_normal,
    Reference_not_found,
    Reference_noindex
};

#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
enum HeadState
{
    Empty,
    Compressed,
    Uncompressed
};
#endif

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
    int			DocID()				{return docID;}
    char		*DocURL()			{return docURL;}
    time_t		DocTime()			{return docTime;}
    char		*DocTitle()			{return docTitle;}
    char		*DocHead();
    char                *DocMetaDsc()                   {return docMetaDsc;}
    time_t		DocAccessed()			{return docAccessed;}
    int			DocLinks()			{return docLinks;}
    int                 DocBackLinks()                  {return docBackLinks;}
    List		*Descriptions()			{return &descriptions;}
    ReferenceState	DocState()			{return docState;}
    int			DocSize()			{return docSize;}
    int			DocImageSize()			{return docImageSize;}
    List		*DocAnchors()			{return &docAnchors;}
    int			DocScore()			{return docScore;}
    int                 DocSig()                        {return docSig;}
    int			DocAnchor()			{return docAnchor;}
    int			DocHopCount()			{return docHopCount;}
    char		*DocEmail()			{return docEmail;}
    char		*DocNotification()		{return docNotification;}
    char		*DocSubject()			{return docSubject;}
	
    void		DocID(int d)			{docID = d;}
    void		DocURL(char *u)			{docURL = u;}
    void		DocTime(time_t t)		{docTime = t;}
    void		DocTitle(char *t)		{docTitle = t;}
    void		DocHead(char *h);
    void                DocMetaDsc(char *md)            {docMetaDsc = md;}
    void		DocAccessed(time_t t)		{docAccessed = t;}
    void		DocLinks(int l)		{docLinks = l;}
    void                DocBackLinks(int l)             {docBackLinks = l;}
    void		Descriptions(List &l)		{descriptions = l;}
    void		AddDescription(char *d);
    void		DocState(ReferenceState s)	{docState = s;}
    void		DocSize(int s)			{docSize = s;}
    void		DocImageSize(int s)		{docImageSize = s;}
    void                DocSig(int s)                   {docSig = s;}
    void		DocAnchors(List &l)		{docAnchors = l;}
    void		AddAnchor(char *a);
    void		DocScore(int s)		{docScore = s;}
    void		DocAnchor(int a)		{docAnchor = a;}
    void		DocHopCount(int h)		{docHopCount = h;}
    void		DocEmail(char *e)		{docEmail = e;}
    void		DocNotification(char *n)	{docNotification = n;}
    void		DocSubject(char *s)		{docSubject = s;}
	
    void		Clear();			// Reset everything

    protected:
    //
    // These values will be stored when serializing
    //

    // This is the index number of the document in the database.
    int			docID;
    // This is the URL of the document.
    String		docURL;
    // This is the time specified in the document's header
    // Usually that's the last modified time, for servers that return it.
    time_t		docTime;
    // This is the time that the last retrieval occurred.
    time_t		docAccessed;
    // This is the stored excerpt of the document, just text.
    String		docHead;
    // This is the document-specified description.
    // For HTML, that's the META description tag.
    String              docMetaDsc;
    // This is the title of the document.
    String		docTitle;
    // This is a list of Strings, the text of links pointing to this document.
    // (e.g. <a href="docURL">description</a>
    List		descriptions;
    // This is the state of the document--modified, normal, etc.
    ReferenceState	docState;
    // This is the size of the original document.
    int			docSize;
    // This is a count of the links in the document (outgoing links).
    int			docLinks;
    // This is a count of the links to the document (incoming links).
    int                 docBackLinks;
    // This is the size of the document when including images.
    int			docImageSize;
    // This is a list of the anchors in the document (i.e. <A NAME=...)
    List		docAnchors;
    // This is a count of the number of hops from start_urls to here.
    int			docHopCount;
    // This is a signature of the document. (e.g. md5sum, checksum...)
    // This is currently unused.
    long int                 docSig;
	
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
    int			docScore;
    // This is the nearest anchor for the search word.
    int			docAnchor;
#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
    //
    // Compression functions
    //
    //static unsigned char c_buffer[32000];
    int Compress(String& s);
    int Decompress(String &s);
    HeadState docHeadState;
#endif
};

#endif


