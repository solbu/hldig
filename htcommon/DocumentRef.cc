//
// DocumentRef.cc
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
// $Id: DocumentRef.cc,v 1.53.2.3 2005/11/28 18:11:42 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "DocumentRef.h"
#include "good_strtok.h"
// Anthony - remove htword stuff
//#include "WordRecord.h"
//#include "WordType.h"
//#include "HtWordReference.h"
#include "HtConfiguration.h"
#include "HtURLCodec.h"
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_STD
  #include <fstream>
  #ifdef HAVE_NAMESPACES
    using namespace std;
  #endif
#else
  #include <fstream.h>
#endif /* HAVE_STD */

// extern HtConfiguration config;

//*****************************************************************************
// DocumentRef::DocumentRef()
//
DocumentRef::DocumentRef()
{
    initialize();
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
  docTime = 0;
  docAccessed = 0;
  docHead = 0;
  docHeadIsSet = 0;
  docMetaDsc = 0;
  docTitle = 0;
  descriptions.Destroy();
  docState = Reference_normal;
  docSize = 0;
  docLinks = 0;
  docBackLinks = 0;
  docAnchors.Destroy();
  docHopCount = 0;
  docSig = 0;
  docEmail = 0;
  docNotification = 0;
  docSubject = 0;
  docScore = 0;
  docAnchor = 0;
}

//*****************************************************************************
// void DocumentRef::DocState(int s)
//
void DocumentRef::DocState(int s)
{
  // You can't easily do this with a cast, so we'll use a switch
  switch(s)
    {
      case 0:
	docState = Reference_normal;
	break;
      case 1:
	docState = Reference_not_found;
	break;
      case 2:
	docState = Reference_noindex;
	break;
      case 3:
	docState = Reference_obsolete;
	break;
    }
}


enum
{
    DOC_ID,				// 0
    DOC_TIME,				// 1
    DOC_ACCESSED,			// 2
    DOC_STATE,				// 3
    DOC_SIZE,				// 4
    DOC_LINKS,				// 5
    DOC_IMAGESIZE,			// 6 -- No longer used
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

// Must be powers of two never reached by the DOC_... enums.
#define CHARSIZE_MARKER_BIT 64
#define SHORTSIZE_MARKER_BIT 128

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
#define addnum(id, out, var) \
 if (var != 0)                                                        \
 {                                                                    \
   if (var <= (unsigned char) ~1)                                     \
   {                                                                  \
     unsigned char _tmp = var;                                        \
     out << (char) (id | CHARSIZE_MARKER_BIT);                        \
     out.append((char *) &_tmp, sizeof(_tmp));                        \
   }                                                                  \
   else if (var <= (unsigned short int) ~1)                           \
   {                                                                  \
     unsigned short int _tmp = var;                                   \
     out << (char) (id | SHORTSIZE_MARKER_BIT);                       \
     out.append((char *) &_tmp, sizeof(_tmp));                        \
   }                                                                  \
   else                                                               \
   {                                                                  \
     out << (char) id;                                                \
     out.append((char *) &var, sizeof(var));                          \
   }                                                                  \
 }

#define	addstring(id, out, str)	\
 if (str.length())                                                    \
 {                                                                    \
   length = str.length();                                             \
   if (length <= (unsigned char) ~1)                                  \
   {                                                                  \
     unsigned char _tmp = length;                                     \
     out << (char) (id | CHARSIZE_MARKER_BIT);                        \
     out.append((char *) &_tmp, sizeof(_tmp));                        \
   }                                                                  \
   else if (length <= (unsigned short int) ~1)                        \
   {                                                                  \
     unsigned short int _tmp = length;                                \
     out << (char) (id | SHORTSIZE_MARKER_BIT);                       \
     out.append((char *) &_tmp, sizeof(_tmp));                        \
   }                                                                  \
   else                                                               \
   {                                                                  \
     out << (char) id;                                                \
     out.append((char *) &length, sizeof(length));                    \
   }                                                                  \
   out.append(str);                                                   \
 }

// To keep compatibility with old databases, don't bother
// with long lists at all.  Bloat the size for long strings with
// one char to just keep a ~1 marker since we don't know the
// endianness; we don't know where to put a endian-safe
// size-marker, and we probably rather want the full char to
// keep the length.  Only strings shorter than (unsigned char) ~1 
// will be "optimized"; trying to optimize strings that fit in
// (unsigned short) does not seem to give anything substantial.
#define	addlist(id, out, list) \
 if (list.Count())                                                    \
 {                                                                    \
   length = list.Count();                                             \
   if (length <= (unsigned short int) ~1)                             \
   {                                                                  \
     if (length <= (unsigned char) ~1)                                \
     {                                                                \
       unsigned char _tmp = length;                                   \
       out << (char) (id | CHARSIZE_MARKER_BIT);                      \
       out.append((char *) &_tmp, sizeof(_tmp));                      \
     }                                                                \
     else                                                             \
     {                                                                \
       unsigned short int _tmp = length;                              \
       out << (char) (id | SHORTSIZE_MARKER_BIT);                     \
       out.append((char *) &_tmp, sizeof(_tmp));                      \
     }                                                                \
     list.Start_Get();                                                \
     while ((str = (String *) list.Get_Next()))		              \
     {                                                                \
       length = str->length();                                        \
       if (length < (unsigned char) ~1)                               \
       {                                                              \
         unsigned char _tmp = length;                                 \
         out.append((char*) &_tmp, sizeof(_tmp));                     \
       }                                                              \
       else                                                           \
       {                                                              \
         unsigned char _tmp = ~1;                                     \
         out.append((char*) &_tmp, sizeof(_tmp));                     \
         out.append((char*) &length, sizeof(length));                 \
       }                                                              \
       out.append(*str);                                              \
     }                                                                \
   }                                                                  \
   else                                                               \
   {                                                                  \
     out << (char) id;                                                \
     out.append((char *) &length, sizeof(length));                    \
     list.Start_Get();                                                \
     while ((str = (String *) list.Get_Next()))                       \
     {                                                                \
       length = str->length();                                        \
       out.append((char*) &length, sizeof(length));                   \
       out.append(*str);                                              \
     }                                                                \
   }                                                                  \
 }

    addnum(DOC_ID, s, docID);
    addnum(DOC_TIME, s, docTime);
    addnum(DOC_ACCESSED, s, docAccessed);
    addnum(DOC_STATE, s, docState);
    addnum(DOC_SIZE, s, docSize);
    addnum(DOC_LINKS, s, docLinks);
    addnum(DOC_BACKLINKS, s, docBackLinks);
    addnum(DOC_HOPCOUNT, s, docHopCount);
    addnum(DOC_SIG, s, docSig);

    // Use a temporary since the addstring macro will evaluate
    // this multiple times.
    String tmps = HtURLCodec::instance()->encode(docURL);
    addstring(DOC_URL, s, tmps);
    // This is done in the DocumentDB code through the excerpt database
    //    addstring(DOC_HEAD, s, docHead);
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
    Clear();
    char	*s = stream.get();
    char	*end = s + stream.length();
    int		length;
    int		count;
    int		i;
    int		x;
    int		throwaway; // As the name sounds--used for old fields
    String	*str;

// There is a problem with getting a numeric value into a
// numeric unknown type that may be an enum (the other way
// around is simply by casting (int)).
//  Supposedly the enum incarnates as a simple type, so we can
// just check the size and copy the bits.
#define MEMCPY_ASSIGN(to, from, type) \
 do {                                                                 \
   type _tmp = (type) (from);                                         \
   memcpy((char *) &(to), (char *) &_tmp, sizeof(to));                \
 } while (0)

#define NUM_ASSIGN(to, from) \
 do {                                                                 \
   if (sizeof(to) == sizeof(unsigned long int))                       \
     MEMCPY_ASSIGN(to, from, unsigned long int);                      \
   else if (sizeof(to) == sizeof(unsigned int))                       \
     MEMCPY_ASSIGN(to, from, unsigned int);                           \
   else if (sizeof(to) == sizeof(unsigned short int))                 \
     MEMCPY_ASSIGN(to, from, unsigned short int);                     \
   else if (sizeof(to) == sizeof(unsigned char))                      \
     MEMCPY_ASSIGN(to, from, unsigned char);                          \
   /* else fatal error here? */                                       \
 } while (0)

#define	getnum(type, in, var) \
 if (type & CHARSIZE_MARKER_BIT)                                      \
 {                                                                    \
   NUM_ASSIGN(var, *(unsigned char *) in);                            \
   in += sizeof(unsigned char);                                       \
 }                                                                    \
 else if (type & SHORTSIZE_MARKER_BIT)                                \
 {                                                                    \
   unsigned short int _tmp0;                                          \
   memcpy((char *) &_tmp0, (char *) (in), sizeof(unsigned short));    \
   NUM_ASSIGN(var, _tmp0);                                            \
   in += sizeof(unsigned short int);                                  \
 }                                                                    \
 else                                                                 \
 {                                                                    \
   memcpy((char *) &var, in, sizeof(var));                            \
   in += sizeof(var);                                                 \
 }

#define	getstring(type, in, str) \
 getnum(type, in, length);                                            \
 str = 0;                                                             \
 str.append(in, length);                                              \
 in += length

#define	getlist(type, in, list) \
 getnum(type, in, count);                                             \
 if (type & (CHARSIZE_MARKER_BIT | SHORTSIZE_MARKER_BIT))             \
 {                                                                    \
   for (i = 0; i < count; i++)                                        \
   {                                                                  \
     unsigned char _tmp = *(unsigned char *) in;                      \
     in += sizeof(_tmp);                                              \
     if (_tmp < (unsigned char) ~1)                                   \
       length = _tmp;                                                 \
     else                                                             \
       getnum(~(CHARSIZE_MARKER_BIT | SHORTSIZE_MARKER_BIT), in,      \
              length);                                                \
     str = new String;                                                \
     str->append(in, length);                                         \
     list.Add(str);                                                   \
     in += length;                                                    \
   }                                                                  \
 }                                                                    \
 else                                                                 \
 {                                                                    \
   for (i = 0; i < count; i++)                                        \
   {                                                                  \
     getnum(~(CHARSIZE_MARKER_BIT | SHORTSIZE_MARKER_BIT), in,        \
            length);                                                  \
     str = new String;                                                \
     str->append(in, length);                                         \
     list.Add(str);                                                   \
     in += length;                                                    \
   }                                                                  \
 }

    while (s < end)
    {
        x = (unsigned char) *s++;
        switch (x & ~(CHARSIZE_MARKER_BIT | SHORTSIZE_MARKER_BIT))
        {
        case DOC_ID:
            getnum(x, s, docID);
            break;
        case DOC_TIME:
            getnum(x, s, docTime);
            break;
        case DOC_ACCESSED:
            getnum(x, s, docAccessed);
            break;
        case DOC_STATE:
            getnum(x, s, docState);
            break;
        case DOC_SIZE:
            getnum(x, s, docSize);
            break;
        case DOC_IMAGESIZE: // No longer used
	    getnum(x, s, throwaway);
	    break;
        case DOC_LINKS:
            getnum(x, s, docLinks);
            break;
        case DOC_HOPCOUNT:
            getnum(x, s, docHopCount);
            break;
	case DOC_BACKLINKS:
	    getnum(x, s, docBackLinks);
	    break;
	case DOC_SIG:
	    getnum(x, s, docSig);
	    break;
        case DOC_URL:
	    {
	      // Use a temporary since the addstring macro will evaluate
	      // this multiple times.
	      String tmps;
	      getstring(x, s, tmps);

	      docURL = HtURLCodec::instance()->decode(tmps);
	    }
	    break;
        case DOC_HEAD:
            getstring(x, s, docHead); docHeadIsSet = 1;
            break;
	case DOC_METADSC:
	    getstring(x, s, docMetaDsc);
	    break;
        case DOC_TITLE:
            getstring(x, s, docTitle);
            break;
        case DOC_DESCRIPTIONS:
            getlist(x, s, descriptions);
            break;
        case DOC_ANCHORS:
            getlist(x, s, docAnchors);
            break;
        case DOC_EMAIL:
            getstring(x, s, docEmail);
            break;
        case DOC_NOTIFICATION:
            getstring(x, s, docNotification);
            break;
        case DOC_SUBJECT:
            getstring(x, s, docSubject);
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
// void DocumentRef::AddDescription(char *d, HtWordList &words)
//
// Anthony - remove since not used any longer, and also uses htword crap
/*
void DocumentRef::AddDescription(const char *d, HtWordList &words)
{
    if (!d || !*d)
        return;

    while (isspace(*d))
        d++;
   
   if (!d || !*d)
     return;

    String	desc = d;
    desc.chop(" \t");

    // Add the description text to the word database with proper factor
    // Do this first because we may have reached the max_description limit
    // This also ensures we keep the proper weight on descriptions 
    // that occur many times

    // Parse words.
    char         *p                   = desc;
    HtConfiguration* config= HtConfiguration::config();
    static int    minimum_word_length = config->Value("minimum_word_length", 3);
    static int    max_descriptions    = config->Value("max_descriptions", 5);

    String word;
    HtWordReference wordRef;
    wordRef.Flags(FLAG_LINK_TEXT);
    wordRef.DocID(docID);

    while (*p)
    {
      // Reset contents before adding chars each round.
      word = 0;

      while (*p && HtIsWordChar(*p))
        word << *p++;

      HtStripPunctuation(word);

      if (word.length() >= minimum_word_length) {
        // The wordlist takes care of lowercasing; just add it.
	wordRef.Location((p - (char*)desc) - word.length());
	wordRef.Word(word);
        words.Replace(wordRef);
      }

      while (*p && !HtIsStrictWordChar(*p))
        p++;
    }

    // And let's flush the words! (nice comment hu :-)
    words.Flush();
    
    // Now are we at the max_description limit?
    if (descriptions.Count() >= max_descriptions)
  	return;
  	
    descriptions.Start_Get();
    String	*description;
    while ((description = (String *) descriptions.Get_Next()))
    {
        if (mystrcasecmp(description->get(), (char*)desc) == 0)
            return;
    }
    descriptions.Add(new String(desc));
}
*/


//*****************************************************************************
// void DocumentRef::AddAnchor(char *a)
//
void DocumentRef::AddAnchor(const char *a)
{
    if (a)
    	docAnchors.Add(new String(a));
}






//****************************************************************
// void DocumentRef::initialize()
//
// Set up the indexDocument with the required hash fields. If these
// aren't set up, the cppAPI won't accept them (hopefully). Also,
// delete anything that might still be in the document.
//
// NOTE: this can be improved... we should probably use an enum
// here instead of all these silly strings. mmm... silly string.
// 
void DocumentRef::initialize()
{
    indexDoc.clear();
    uniqueWords.clear();

    indexDoc["url"].second = "Keyword";
    indexDoc["title"].second = "Keyword";
    indexDoc["author"].second = "Keyword";
    indexDoc["head"].second = "Keyword";
    indexDoc["meta_desc"].second = "Keyword";
    indexDoc["meta_email"].second = "Keyword";
    indexDoc["meta_notification"].second = "Keyword";
    indexDoc["meta_subject"].second = "Keyword";

    indexDoc["contents"].second = "UnStored";
    
    indexDoc["time"].second = "UnIndexed";
}


//*****************************************************************
// void DocumentRef::dumpUniqueWords()
//
// Take the unique words that have been stored so far and append each
// to the contents field of the indexDoc. then clear out the unique words.
// 
// NOTE: will need modification when unicode goes in (might be able to
// replace with a bunch of appendField calls)
// 
void DocumentRef::dumpUniqueWords()
{
    uniqueWordsSet::iterator i;
    for (i = uniqueWords.begin(); i != uniqueWords.end(); i++) {
        indexDoc["contents"].first.insert(indexDoc["contents"].first.size(), *i);
        indexDoc["contents"].first.push_back(' ');
    }
    uniqueWords.clear();
}


//*****************************************************************
// void addUniqueWord(const char* word)
//
// add a unique word to the uniqueWords set
//
// NOTE: will need modification when unicode goes in
// 
void DocumentRef::addUniqueWord(char* word)
{
    uniqueWords.insert(word);
}


//*****************************************************************
// void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
//
// Take the specified field text/value and insert into the indexDoc
// at the specified field
//
// NOTE: will need modification when unicode goes in, the field value
// will most likely be a string object
// 
void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
{
    indexDoc[fieldName].first = fieldValue;
}


//*****************************************************************
// void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
//
// Append the specified field text/value to the specified field
//
// NOTE: will need modificatioin when Unicode goes in (the separator
// will need to be a wide character)
//
void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
{
    indexDoc[fieldName].first.insert(indexDoc[fieldName].first.size(), fieldValue);
    indexDoc[fieldName].first.push_back(' ');
}


