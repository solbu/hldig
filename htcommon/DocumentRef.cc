//
// DocumentRef.cc
//
// Implementation of DocumentRef
//
// $Log: DocumentRef.cc,v $
// Revision 1.5  1998/10/18 20:37:41  ghutchis
//
// Fixed database corruption bug and other misc. cleanups.
//
// Revision 1.4  1998/08/11 08:58:24  ghutchis
// Second patch for META description tags. New field in DocDB for the
// desc., space in word DB w/ proper factor.
//
// Revision 1.3  1998/01/05 00:49:16  turtle
// format changes
//
// Revision 1.2  1997/02/10 17:30:58  turtle
// Applied AIX specific patches supplied by Lars-Owe Ivarsson
// <lars-owe.ivarsson@its.uu.se>
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
// Revision 1.1  1995/07/06 23:43:12  turtle
// *** empty log message ***
//
//

#include "DocumentRef.h"
#include <good_strtok.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>


//*****************************************************************************
// DocumentRef::DocumentRef()
//
DocumentRef::DocumentRef()
{
    Clear();
}


//*****************************************************************************
// DocumentRef::~DocumentRef()
//
DocumentRef::~DocumentRef()
{
}


//*****************************************************************************
// void DocumentRef::Clear()
//
void DocumentRef::Clear()
{
    docID = 0;
    docURL = 0;
    docTitle = 0;
    docState = Reference_normal;
    docTime = 0;
    docSize = 0;
    docImageSize = 0;
    docHead = 0;
    docMetaDsc = 0;
    docAccessed = 0;
    docLinks = 0;
    descriptions.Destroy();
    docAnchors.Destroy();
    docHopCount = -1;
}


enum
{
    DOC_ID,				// 0
    DOC_TIME,				// 1
    DOC_ACCESSED,			// 2
    DOC_STATE,				// 3
    DOC_SIZE,				// 4
    DOC_LINKS,				// 5
    DOC_IMAGESIZE,			// 6
    DOC_HOPCOUNT,			// 7
    DOC_URL,				// 8
    DOC_HEAD,				// 9
    DOC_TITLE,				// 10
    DOC_DESCRIPTIONS,	        	// 11
    DOC_ANCHORS,			// 12
    DOC_EMAIL,				// 13
    DOC_NOTIFICATION,		        // 14
    DOC_SUBJECT,			// 15
    DOC_STRING,                         // 16
    DOC_METADSC,                        // 17
};


//*****************************************************************************
// void DocumentRef::Serialize(String &s)
//   Convert all the data in the object to a string. 
//   The data is in the string is tagged with 
//
void DocumentRef::Serialize(String &s)
{
    int		length;
    String	*str;

//
// The following macros make the serialization process a little easier
// to follow.  Note that if an object to be serialized has the default
// value for this class, it it NOT serialized.  This means that
// storage will be saved...
//
#define addnum(id, out, var)	if (var != 0)										\
    {													\
                                                                                                            out << (char) id;								\
    out.append((char *) &var, sizeof(var));			\
    }
#define	addstring(id, out, str)	if (str.length())									\
    {													\
                                                                                                            length = str.length();							\
    out << (char) id;								\
    out.append((char *) &length, sizeof(length));	\
    out.append(str);								\
    }
#define	addlist(id, out, list)	if (list.Count())									\
    {													\
                                                                                                            length = list.Count();							\
    out << (char) id;								\
    out.append((char *) &length, sizeof(length));	\
    list.Start_Get();								\
    while ((str = (String *) list.Get_Next()))		\
        {												\
                                                                                                            length = str->length();						\
        out.append((char*) &length, sizeof(length));\
        out.append(*str);							\
        }												\
    }

    addnum(DOC_ID, s, docID);
    addnum(DOC_TIME, s, docTime);
    addnum(DOC_ACCESSED, s, docAccessed);
    addnum(DOC_STATE, s, docState);
    addnum(DOC_SIZE, s, docSize);
    addnum(DOC_LINKS, s, docLinks);
    addnum(DOC_IMAGESIZE, s, docImageSize);
    addnum(DOC_HOPCOUNT, s, docHopCount);

    addstring(DOC_URL, s, docURL);
    addstring(DOC_HEAD, s, docHead);
    addstring(DOC_METADSC, s, docMetaDsc);
    addstring(DOC_TITLE, s, docTitle);

    addlist(DOC_DESCRIPTIONS, s, descriptions);
    addlist(DOC_ANCHORS, s, docAnchors);

    addstring(DOC_EMAIL, s, docEmail);
    addstring(DOC_NOTIFICATION, s, docNotification);
    addstring(DOC_SUBJECT, s, docSubject);
}


//*****************************************************************************
// void DocumentRef::Deserialize(String &stream)
//   Extract the contents of our private variables from the given
//   character string.  The character string is expected to have been
//   created using the Serialize member.
//
void DocumentRef::Deserialize(String &stream)
{
    char	*s = stream.get();
    char	*end = s + stream.length();
    int		length;
    int		count;
    int		i;
    int		x;
    String	*str;

    Clear();

#define	getnum(in, var)			memcpy((char *) &var, in, sizeof(var));			\
    in += sizeof(var)
#define	getstring(in, str)		getnum(in, length);								\
        str = 0;										\
    str.append(in, length);							\
    in += length
#define	getlist(in, list)		getnum(in, count);								\
                                                                                    for (i = 0; i < count; i++)						\
        {												\
                                                                                                            getnum(in, length);							\
        str = new String;							\
        str->append(in, length);					\
        list.Add(str);								\
        in += length;								\
        }

    while (s < end)
    {
        x = *s++;
        switch (x)
        {
        case DOC_ID:
            getnum(s, docID);
            break;
        case DOC_TIME:
            getnum(s, docTime);
            break;
        case DOC_ACCESSED:
            getnum(s, docAccessed);
            break;
        case DOC_STATE:
            getnum(s, docState);
            break;
        case DOC_SIZE:
            getnum(s, docSize);
            break;
        case DOC_LINKS:
            getnum(s, docLinks);
            break;
        case DOC_IMAGESIZE:
            getnum(s, docImageSize);
            break;
        case DOC_HOPCOUNT:
            getnum(s, docHopCount);
            break;
        case DOC_URL:
            getstring(s, docURL);
            break;
        case DOC_HEAD:
            getstring(s, docHead);
            break;
	case DOC_METADSC:
	    getstring(s, docMetaDsc);
	    break;
        case DOC_TITLE:
            getstring(s, docTitle);
            break;
        case DOC_DESCRIPTIONS:
            getlist(s, descriptions);
            break;
        case DOC_ANCHORS:
            getlist(s, docAnchors);
            break;
        case DOC_EMAIL:
            getstring(s, docEmail);
            break;
        case DOC_NOTIFICATION:
            getstring(s, docNotification);
            break;
        case DOC_SUBJECT:
            getstring(s, docSubject);
            break;
	case DOC_STRING:
	  // This is just a debugging string. Ignore it.
	    break;
        default:
            cerr << "BAD TAG IN SERIALIZED DATA: " << x << endl;
            return;
        }
    }
}


//*****************************************************************************
// void DocumentRef::AddDescription(char *d)
//
void DocumentRef::AddDescription(char *d)
{
    if (!d || descriptions.Count() >= 5)
        return;

    while (isspace(*d))
        d++;

    String	desc = d;
    desc.chop(" \t");

    descriptions.Start_Get();
    String	*description;
    while ((description = (String *) descriptions.Get_Next()))
    {
        if (mystrcasecmp(description->get(), desc) == 0)
            return;
    }
    descriptions.Add(new String(desc));
}


//*****************************************************************************
// void DocumentRef::AddAnchor(char *a)
//
void DocumentRef::AddAnchor(char *a)
{
    docAnchors.Add(new String(a));
}


