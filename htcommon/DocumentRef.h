//
// DocumentRef.h
//
// $Id: DocumentRef.h,v 1.9 1999/01/05 19:29:40 ghutchis Exp $
//
// $Log: DocumentRef.h,v $
// Revision 1.9  1999/01/05 19:29:40  ghutchis
// Added comments on the members (fields) of DocumentRef objects.
//
// Revision 1.8  1998/11/15 22:29:27  ghutchis
// Implement docBackLinks backlink count.
//
// Revision 1.7  1998/09/10 04:16:25  ghutchis
// More bug fixes.
//
// Revision 1.6  1998/09/07 04:37:16  ghutchis
// Added DocState for documents marked as "noindex".
//
// Revision 1.5  1998/08/11 08:58:25  ghutchis
// Second patch for META description tags. New field in DocDB for the
// desc., space in word DB w/ proper factor.
//
// Revision 1.4  1998/01/05 00:50:30  turtle
// format changes
//
// Revision 1.3  1997/03/24 04:33:15  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.2  1997/02/10 17:30:58  turtle
// Applied AIX specific patches supplied by Lars-Owe Ivarsson
// <lars-owe.ivarsson@its.uu.se>
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
// Revision 1.1  1995/07/06 23:44:00  turtle
// *** empty log message ***
//
//
#ifndef _DocumentRef_h_
#define _DocumentRef_h_

#include <htString.h>
#include <List.h>
#include <time.h>

enum ReferenceState
{
    Reference_normal,
    Reference_not_found,
    Reference_noindex
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
    int			DocID()				{return docID;}
    char		*DocURL()			{return docURL;}
    time_t		DocTime()			{return docTime;}
    char		*DocTitle()			{return docTitle;}
    char		*DocHead()			{return docHead;}
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
    void		DocHead(char *h)		{docHead = h;}
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
};

#endif


