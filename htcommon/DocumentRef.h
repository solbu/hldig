//
// DocumentRef.h
//
// $Id: DocumentRef.h,v 1.4 1998/01/05 00:50:30 turtle Exp $
//
// $Log: DocumentRef.h,v $
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
    Reference_not_found
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
    time_t		DocAccessed()			{return docAccessed;}
    int			DocLinks()			{return docLinks;}
    List		*Descriptions()			{return &descriptions;}
    ReferenceState	DocState()			{return docState;}
    int			DocSize()			{return docSize;}
    int			DocImageSize()			{return docImageSize;}
    List		*DocAnchors()			{return &docAnchors;}
    int			DocScore()			{return docScore;}
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
    void		DocAccessed(time_t t)		{docAccessed = t;}
    void		DocLinks(int l)		{docLinks = l;}
    void		Descriptions(List &l)		{descriptions = l;}
    void		AddDescription(char *d);
    void		DocState(ReferenceState s)	{docState = s;}
    void		DocSize(int s)			{docSize = s;}
    void		DocImageSize(int s)		{docImageSize = s;}
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
    int			docID;
    String		docURL;
    time_t		docTime;
    time_t		docAccessed;
    String		docHead;
    String		docTitle;
    List		descriptions;
    ReferenceState	docState;
    int			docSize;
    int			docLinks;
    int			docImageSize;
    List		docAnchors;
    int			docHopCount;
	
    //
    // The following values are for the email notification of expiration
    //
    String		docEmail;
    String		docNotification;
    String		docSubject;

    //
    // This is used for searching and is not stored in the database
    //
    int			docScore;
    int			docAnchor;
};

#endif


