//
// DocumentRef.cc
//
// Implementation of DocumentRef
// Reference to an indexed document. Keeps track of all information stored
// on the document, either by the dig or temporary search information.
//
//

#include "DocumentRef.h"
#include "good_strtok.h"
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>
#include "WordList.h"
#include "Configuration.h"
#include "HtURLCodec.h"

#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
#include <zlib.h>
#endif

extern Configuration config;

#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
//unsigned char DocumentRef::c_buffer[32000];
//
// Compress Function
//
int DocumentRef::Compress(String &s) {
  static int cf=config.Value("compression_level",0);    
  if (cf) {
    //
    // Now compress s into c_s
    //
    unsigned char c_buffer[16384];
    String c_s;
    z_stream c_stream; /* compression stream */
    c_stream.zalloc=(alloc_func)0;
    c_stream.zfree=(free_func)0;
    c_stream.opaque=(voidpf)0;
    // Get compression factor, default to best
    if (cf<-1) cf=-1; else if (cf>9) cf=9;
    int err=deflateInit(&c_stream,cf);
    if (err!=Z_OK) return 0;
    int len=s.length();
    c_stream.next_in=(Bytef*)(char *)s;
    c_stream.avail_in=len;
    while (err==Z_OK && c_stream.total_in!=(uLong)len) {
      c_stream.next_out=c_buffer;
      c_stream.avail_out=sizeof(c_buffer);
      err=deflate(&c_stream,Z_NO_FLUSH);
      c_s.append((char *)c_buffer,c_stream.next_out-c_buffer);
    }
    // Finish the stream
    for (;;) {
      c_stream.next_out=c_buffer;
      c_stream.avail_out=sizeof(c_buffer);
      err=deflate(&c_stream,Z_FINISH);
      c_s.append((char *)c_buffer,c_stream.next_out-c_buffer);
      if (err==Z_STREAM_END) break;
      //CHECK_ERR(err, "deflate");
    }
    err=deflateEnd(&c_stream); 
    s=c_s;
  }
  return 1;
}

//
// Decompress routine returns 0 if decompressed 1 if compressed
//
int DocumentRef::Decompress(String &s) {
  static int cf=config.Value("compression_level",0);    
  if (cf) {
    String c_s;
    // Decompress stream
    unsigned char c_buffer[16384];
    z_stream d_stream;
    d_stream.zalloc=(alloc_func)0;
    d_stream.zfree=(free_func)0;
    d_stream.opaque=(voidpf)0;
    
    int len=s.length();
    d_stream.next_in=(Bytef*)(char *)s;
    d_stream.avail_in=len;
    
    int err=inflateInit(&d_stream);
    if (err!=Z_OK) return 1;
    
    while (err==Z_OK && d_stream.total_in<len) {
      d_stream.next_out=c_buffer;
      d_stream.avail_out=sizeof(c_buffer);
      err=inflate(&d_stream,Z_NO_FLUSH);
      c_s.append((char *)c_buffer,d_stream.next_out-c_buffer);
      if (err==Z_STREAM_END) break;
    }
    
    err=inflateEnd(&d_stream);
    s=c_s;
  }
  return 0;
}

char *DocumentRef::DocHead() {
  if (docHeadState==Compressed) {
    Decompress(docHead);
    docHeadState=Uncompressed;
  }
  return docHead;
}

void DocumentRef::DocHead(char *h) {
  docHead=h;
  docHeadState=docHead.length()==0?Empty:Uncompressed;
}
#else
 
char *DocumentRef::DocHead() {
  return docHead;
}

void DocumentRef::DocHead(char *h) {
  docHead=h;
}
#endif

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
#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
    docHeadState=Empty;
#endif
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

#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
    if (docHeadState==Uncompressed) {
      Compress(docHead);
      docHeadState=Compressed;
    }
#endif
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
    addnum(DOC_IMAGESIZE, s, docImageSize);
    addnum(DOC_HOPCOUNT, s, docHopCount);
    addnum(DOC_SIG, s, docSig);

    // Use a temporary since the addstring macro will evaluate
    // this multiple times.
    String tmps = HtURLCodec::instance()->encode(docURL);
    addstring(DOC_URL, s, tmps);
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
    Clear();
    char	*s = stream.get();
    char	*end = s + stream.length();
    int		length;
    int		count;
    int		i;
    int		x;
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
        case DOC_LINKS:
            getnum(x, s, docLinks);
            break;
        case DOC_IMAGESIZE:
            getnum(x, s, docImageSize);
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
            getstring(x, s, docHead);
#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
            docHeadState=docHead.length()==0?Empty:Compressed;
#endif
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
// void DocumentRef::AddDescription(char *d)
//
void DocumentRef::AddDescription(char *d)
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

    static WordList *words = 0;
    
    if (!words) // Hey... We only want to do this once, right?
    {
	words = new WordList();
	words->WordTempFile(config["word_list"]);
	words->BadWordFile(config["bad_word_list"]);
    }

    words->DocumentID(docID);
    
    // Parse words, taking care of valid_punctuation.
    char         *p                   = desc;
    static char  *valid_punctuation   = config["valid_punctuation"];
    static int    minimum_word_length = config.Value("minimum_word_length", 3);
    static double description_factor  = config.Double("description_factor");
    static int    max_descriptions    = config.Value("max_descriptions", 5);

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
        words->Word(word, 0, 0, description_factor);

      // No need to count in valid_punctuation for the beginning-char.
      while (*p && !isalnum(*p))
        p++;
    }

    // And let's flush the words!
    words->Flush();
    
    // Now are we at the max_description limit?
    if (descriptions.Count() >= max_descriptions)
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


