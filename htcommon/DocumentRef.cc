//
// DocumentRef.cc
//
// Implementation of DocumentRef
//
// $Log: DocumentRef.cc,v $
// Revision 1.14  1999/01/06 18:21:16  ghutchis
// Applied fix from Dave Alden <alden@math.ohio-state.edu> to compile under
// SunPRO compilers by eliminating trailing comma in enum.
//
// Revision 1.13  1999/01/06 15:39:47  ghutchis
// Remove delete instruction that fouls up everything (it was removing
// descriptions as we add them!).
//
// Revision 1.12  1999/01/06 05:42:12  ghutchis
// Do not add non-word characters to the wordlist.
//
// Revision 1.11  1999/01/05 19:35:42  ghutchis
// Fix dereferencing mistake in last version.
//
// Revision 1.9  1998/12/08 02:52:50  ghutchis
// Fix typo that added description text that contained punctuation or was too
// short.
//
// Revision 1.8  1998/12/06 18:44:43  ghutchis
// Add the text of descriptions to the word database with weight
// description_factor.
//
// Revision 1.7  1998/11/18 05:16:28  ghutchis
// Remove limit on link descriptions.
//
// Revision 1.6  1998/11/15 22:29:27  ghutchis
// Implement docBackLinks backlink count.
//
// Revision 1.5  1998/10/18 20:37:41  ghutchis
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
#include "WordList.h"
#include <Configuration.h>

extern Configuration config;

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
    docBackLinks = 0;
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
    DOC_BACKLINKS,                      // 18
    DOC_SIG                             // 19
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
    addnum(DOC_BACKLINKS, s, docBackLinks);
    addnum(DOC_IMAGESIZE, s, docImageSize);
    addnum(DOC_HOPCOUNT, s, docHopCount);
    addnum(DOC_SIG, s, docSig);

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
	case DOC_BACKLINKS:
	    getnum(s, docBackLinks);
	    break;
	case DOC_SIG:
	    getnum(s, docSig);
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
    if (!d)
        return;

    while (isspace(*d))
        d++;

    String	desc = d;
    desc.chop(" \t");

    // Add the description text to the word database with proper factor
    // Do this first because we may have reached the max_description limit
    // This also ensures we keep the proper weight on descriptions 
    // that occur many times

    static WordList *words = 0;
    
    if (!words) // Hey... We only want to do this once, right?
    {
	words = new WordList();
	words->WordTempFile(config["word_list"]);
	words->BadWordFile(config["bad_word_list"]);
    }

    words->DocumentID(docID);
    
    // Parse words, taking care of valid_punctuation.
    char *p                   = desc;
    char *valid_punctuation   = config["valid_punctuation"];
    int   minimum_word_length = config.Value("minimum_word_length", 3);

    // Not restricted to this size, just used as a hint.
    String word(MAX_WORD_LENGTH);

    if (!valid_punctuation)
	valid_punctuation = "";

    while (*p)
    {
      // Reset contents before adding chars each round.
      word = 0;

      while (*p && (isalnum(*p) || strchr(valid_punctuation, *p)))
        word << *p++;

      word.remove(valid_punctuation);

      if (word.length() >= minimum_word_length)
        // The wordlist takes care of lowercasing; just add it.
        words->Word(word, 0, 0, config.Double("description_factor"));

      // No need to count in valid_punctuation for the beginning-char.
      while (*p && !isalnum(*p))
        p++;
    }

    // And let's flush the words!
    words->Flush();
    
    // Now are we at the max_description limit?
    if (descriptions.Count() >= config.Value("max_descriptions", 5))
  	return;
  	
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
    if (a)
    	docAnchors.Add(new String(a));
}


