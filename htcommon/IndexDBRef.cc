//
// IndexDBRef.cc
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
// $Id: IndexDBRef.cc,v 1.1.2.3 2005/12/07 19:16:31 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "IndexDBRef.h"
#include "good_strtok.h"
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
// IndexDBRef::IndexDBRef()
//
IndexDBRef::IndexDBRef()
{
    Clear();
}


//*****************************************************************************
// IndexDBRef::~IndexDBRef()
//
IndexDBRef::~IndexDBRef()
{
}


//*****************************************************************************
// void IndexDBRef::Clear()
//
void IndexDBRef::Clear()
{
    URL = 0;
    time = 0;
//    descriptions.Destroy();
    sig = 0;
//    state = Reference_normal;
    backlinks = 0;
    hopCount = 0;
}


enum
{
    DOC_ID,             // 0
    DOC_TIME,           // 1
    DOC_ACCESSED,       // 2
    DOC_STATE,          // 3
    DOC_SIZE,           // 4
    DOC_LINKS,          // 5
    DOC_IMAGESIZE,      // 6 -- No longer used
    DOC_HOPCOUNT,       // 7
    DOC_URL,            // 8
    DOC_HEAD,           // 9
    DOC_TITLE,          // 10
    DOC_DESCRIPTIONS,   // 11
    DOC_ANCHORS,        // 12
    DOC_EMAIL,          // 13
    DOC_NOTIFICATION,   // 14
    DOC_SUBJECT,        // 15
    DOC_STRING,         // 16
    DOC_METADSC,        // 17
    DOC_BACKLINKS,      // 18
    DOC_SIG             // 19
};

// Must be powers of two never reached by the DOC_... enums.
#define CHARSIZE_MARKER_BIT 64
#define SHORTSIZE_MARKER_BIT 128

//*****************************************************************************
// void IndexDBRef::Serialize(String &s)
//   Convert all the data in the object to a string. 
//   The data is in the string is tagged with 
//
void IndexDBRef::Serialize(String &s)
{
//    int		length;
//    String	*str;

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
     while ((str = (String *) list.Get_Next()))                       \
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

    addnum(DOC_TIME, s, time);
    addnum(DOC_BACKLINKS, s, backlinks);
    addnum(DOC_HOPCOUNT, s, hopCount);
    
// NOTE: the sig will go back in when MD5 is done
//
//    addnum(DOC_SIG, s, docSig);

// NOTE: descriptions will be going back in eventually
// 
//    addlist(DOC_DESCRIPTIONS, s, descriptions);

}


//*****************************************************************************
// void IndexDBRef::Deserialize(String &stream)
//   Extract the contents of our private variables from the given
//   character string.  The character string is expected to have been
//   created using the Serialize member.
//
void IndexDBRef::Deserialize(String &stream)
{
    Clear();
    char    *s = stream.get();
    char    *end = s + stream.length();
//    int     length;
//    int     count;
//    int     i;
    int     x;
//    int     throwaway; // As the name sounds--used for old fields
//    String  *str;

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
        case DOC_TIME:
            getnum(x, s, time);
            break;
        case DOC_SIG:
            getnum(x, s, sig);
            break;
        case DOC_DESCRIPTIONS:
//            getlist(x, s, descriptions);
            break;
        case DOC_HOPCOUNT:
            getnum(x, s, hopCount);
            break;
        case DOC_BACKLINKS: 
            getnum(x, s, backlinks);
            break;

        default:
            cerr << "BAD TAG IN SERIALIZED DATA: " << x << endl;
            return;
        }
    }
}

