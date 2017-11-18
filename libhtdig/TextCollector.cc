//--------------------------------------------------------------------
//
// TextCollector.cc
//
// 2/6/2002 created for libhtdig
//
// Neal Richter nealr@rightnow.com
//
// TextCollector:
//            General Purpose Text Document Indexer.
//            Calls appropriate parsers. 
//            The  parser notifies the TextCollector object that it got something
//            (got_* functions) and the TextCollector object feed the databases
//            and statistics accordingly.
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: TextCollector.cc,v 1.4 2004/05/28 13:15:29 lha Exp $
//
//--------------------------------------------------------------------


#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "TextCollector.h"
#include "htdig.h"
#include "HtWordList.h"
#include "WordRecord.h"
#include "URLRef.h"
#include "Server.h"
#include "Parsable.h"
#include "BasicDocument.h"
#include "StringList.h"
#include "WordType.h"
#include "md5.h"
#include "defaults.h"

#include <signal.h>
#include <stdio.h>

#include <sys/timeb.h>


//*****************************************************************************
// TextCollector::TextCollector()
//
TextCollector::TextCollector(TextCollectorLog flags):
words(*(HtConfiguration::config()))
{
  HtConfiguration *config = HtConfiguration::config();
  //FILE *urls_parsed;

  currenthopcount = 0;

    //turn on word tracking!
    trackWords = 1;

  //
  // Initialize the flags for the various HTML factors
  //
    
  // text_factor
  factor[0] = FLAG_TEXT;
  // title_factor
  factor[1] = FLAG_TITLE;
  // heading factor (now generic)
  factor[2] = FLAG_HEADING;
  factor[3] = FLAG_HEADING;
  factor[4] = FLAG_HEADING;
  factor[5] = FLAG_HEADING;
  factor[6] = FLAG_HEADING;
  factor[7] = FLAG_HEADING;
  // img alt text
  //factor[8] = FLAG_KEYWORDS;
  factor[8] = FLAG_TEXT;    // treat alt text as plain text, until it has
  // its own FLAG and factor.
  // keywords factor
  factor[9] = FLAG_KEYWORDS;
  // META description factor
  factor[10] = FLAG_DESCRIPTION;

  doc = NULL;
  minimumWordLength = config->Value("minimum_word_length", 3);


  //TODO put document-index log file stuff here via logs like Retriever

  check_unique_md5 = config->Boolean("check_unique_md5", 0);
  check_unique_date = config->Boolean("check_unique_date", 0);

  d_md5 = 0;
  if (check_unique_md5)
  {
    d_md5 = Database::getDatabaseInstance(DB_HASH);

    if (d_md5->OpenReadWrite(config->Find("md5_db"), 0666) != OK)
    {
      cerr << "DocumentDB::Open: " << config->Find("md5_db") << " " << strerror(errno) << "\n";
    }
  }

    temp_doc_count = 0;

}


//*****************************************************************************
// TextCollector::~TextCollector()
//
TextCollector::~TextCollector()
{
  if (d_md5)
    d_md5->Close();
  //delete doc;

    if(temp_doc_count != 0)
    {
        words.Flush();
        temp_doc_count = 0;
    }

    words.Flush();
  words.Close();
    
}


//*****************************************************************************
// void TextCollector::IndexDoc()
//
//

int
TextCollector::IndexDoc(BasicDocument & a_basicdoc)
{
  DocumentRef *ref;
    time_t    date;
    int      old_document = 0;
    static int    index = 0;

    //struct timeb tb;

  //HtConfiguration *config = HtConfiguration::config();

    doc = &a_basicdoc;

  ref = docs[doc->Location()];  // It might be nice to have just an Exists() here
  if (ref)
  {
    //
    // We already have an entry for this document in our database.
    // This means we can get the document ID and last modification
    // time from there.
    //
    current_id = ref->DocID();
    date = ref->DocTime();
    if (ref->DocAccessed())
      old_document = 1;
    else  // we haven't retrieved it yet, so we only have the first link
      old_document = 0;
    ref->DocBackLinks(ref->DocBackLinks() + 1);  // we had a new link
    ref->DocAccessed(time(0));
    ref->DocState(Reference_normal);
    currenthopcount = ref->DocHopCount();
  }
  else
  {
    //
    // Never seen this document before.  We need to create an
    // entry for it.  This implies that it gets a new document ID.
    //

        date = 0;
       
        current_id = docs.NextDocID();
    ref = new DocumentRef;
    ref->DocID(current_id);
    ref->DocURL(doc->Location());
    ref->DocState(Reference_normal);
    ref->DocAccessed(time(0));
    ref->DocHopCount(0);
    ref->DocBackLinks(1); // We had to have a link to get here!
    old_document = 0;
  }

  word_context.DocID(ref->DocID());

  if (debug > 0)
  {
    //
    // Display progress
    //
    cout << index++ << ':' << current_id << ':' << currenthopcount << ':' << doc->Location() <<
      ": ";
    cout.flush();
  }

    //printf("New Doc\n");
    //ftime(&tb);
    //fprintf(stderr, "[1] TIME: [%s] [%d]\n", ctime(&tb.time), tb.millitm);

  RetrievedDocument(ref);

    //ftime(&tb);
    //fprintf(stderr, "[2] TIME: [%s] [%d]\n", ctime(&tb.time), tb.millitm);

    if(temp_doc_count > 250)
    {
        //words.Flush();
        temp_doc_count = 0;
    }
    else
    {
        temp_doc_count++;
    }

    //ftime(&tb);
    //fprintf(stderr, "[3] TIME: [%s] [%d]\n", ctime(&tb.time), tb.millitm);

  docs.Add(*ref);

    //ftime(&tb);
    //fprintf(stderr, "[4] TIME: [%s] [%d]\n", ctime(&tb.time), tb.millitm);

    delete ref;

    words.Flush();
    //words.Close();

    if (urls_seen)
    {
        fprintf(urls_seen, "%s|%d|%s|%d|0|1\n",
                (const char *) doc->Location(), doc->Length(), doc->ContentType(),
                (int) doc->ModTime());
    }

    
    return(1);
}

int TextCollector::FlushWordDB()
{
    if(temp_doc_count != 0)
    {
        words.Flush();
        temp_doc_count = 0;
    }

    words.Flush();
    words.Close();
    return(1);
}
        
//*****************************************************************************
// void TextCollector::RetrievedDocument(Document &doc, const String &url, DocumentRef *ref)
//   We found a document that needs to be parsed.  Since we don't know the
//   document type, we'll let the Document itself return an appropriate
//   Parsable object which we can call upon to parse the document contents.
//
void
TextCollector::RetrievedDocument(DocumentRef * ref)
{
  n_links = 0;
  current_ref = ref;
  current_title = 0;
  word_context.Anchor(0);
  current_time = 0;
  current_head = 0;
  current_meta_dsc = 0;
    time_t doc_time;

    //Check if the Document is self-parseable
    //We will pass ourselves as a callback object for all the got_*() routines
  if (doc->SelfParseable() == TRUE)
  {
    doc->internalParser(*this);
  }
  else
    {
      // Create a parser object and let it have a go at the document.
      // We will pass ourselves as a callback object for all the got_*()
      // routines.
      // This will generate the Parsable object as a specific parser
      /*
    Parsable *parsable = doc->getParsable();
    if (parsable)
      parsable->parse(*this, *base);
    else
    {          // If we didn't get a parser, then we should get rid of this!
      ref->DocState(Reference_noindex);
      return;
    }
        */
  }

  // We don't need to dispose of the parsable object since it will
  // automatically be reused.


  //
  // Update the document reference
  //
  ref->DocTitle((char *) current_title);
  ref->DocHead((char *) current_head);
  ref->DocMetaDsc((char *) current_meta_dsc);
  
/*    if (current_time == 0)
    ref->DocTime(doc->ModTime());
  else
    ref->DocTime(current_time); */
    
    doc_time = doc->ModTime();
    if(doc_time != 0)
        ref->DocTime(doc_time);
    else
        ref->DocTime(time(NULL));
        
  ref->DocSize(doc->Length());
  ref->DocAccessed(time(0));
  ref->DocLinks(n_links);
}


//*****************************************************************************
// void TextCollector::got_word(char *word, int location, int heading)
//   The location is normalized to be in the range 0 - 1000.
//
void
TextCollector::got_word(const char *word, int location, int heading)
{
  if (debug > 3)
    cout << "word: " << word << '@' << location << endl;
  if (heading >= 11 || heading < 0)  // Current limits for headings
    heading = 0;      // Assume it's just normal text

  if ((trackWords) && (strlen(word) >= minimumWordLength))
  {
    String w = word;
    HtWordReference wordRef;

    wordRef.Location(location);
    wordRef.Flags(factor[heading]);

    wordRef.Word(w);
    words.Replace(WordReference::Merge(wordRef, word_context));

#ifdef DEBUG
        cout << "Adding: [" << w <<  "]"<< endl;  //NEALR
#endif
            
    // Check for compound words...
    String parts = word;
    int added;
    int nparts = 1;
    do
    {
      added = 0;
      char *start = parts.get();
      char *punctp = 0, *nextp = 0, *p;
      char punct;
      int n;
      while (*start)
      {
        p = start;
        for (n = 0; n < nparts; n++)
        {
          while (HtIsStrictWordChar((unsigned char) *p))
            p++;
          punctp = p;
          if (!*punctp && n + 1 < nparts)
            break;
          while (*p && !HtIsStrictWordChar((unsigned char) *p))
            p++;
          if (n == 0)
            nextp = p;
        }
        if (n < nparts)
          break;
        punct = *punctp;
        *punctp = '\0';
        if (*start && (*p || start > parts.get()))
        {
          w = start;
          HtStripPunctuation(w);
          if (w.length() >= minimumWordLength)
          {
            wordRef.Word(w);
            words.Replace(WordReference::Merge(wordRef, word_context));
            if (debug > 3)
              cout << "word part: " << start << '@' << location << endl;

#ifdef DEBUG
                        cout << "Adding: [" << w <<  "]"<< endl;  //NEALR
#endif                            
          }
          added++;
        }
        start = nextp;
        *punctp = punct;
      }
      nparts++;
    }
    while (added > 2);
  }
}


//*****************************************************************************
// void TextCollector::got_title(const char *title)
//
void
TextCollector::got_title(const char *title)
{
  if (debug > 1)
    cout << "\ntitle: " << title << endl;
  current_title = title;
}

//*****************************************************************************
// void TextCollector::got_time(const char *time)
//
void
TextCollector::got_time(const char *time)
{
  HtDateTime new_time(current_time);

  if (debug > 1)
    cout << "\ntime: " << time << endl;

  //
  // As defined by the Dublin Core, this should be YYYY-MM-DD
  // In the future, we'll need to deal with the scheme portion
  //  in case someone picks a different format.
  //
  new_time.SetFTime(time, "%Y-%m-%d");
  current_time = new_time.GetTime_t();

  // If we can't convert it, current_time stays the same and we get
  // the default--the date returned by the server...
}

//*****************************************************************************
// void TextCollector::got_head(const char *head)
//
void
TextCollector::got_head(const char *head)
{
  if (debug > 4)
    cout << "head: " << head << endl;
  current_head = head;
}

//*****************************************************************************
// void TextCollector::got_meta_dsc(const char *md)
//
void
TextCollector::got_meta_dsc(const char *md)
{
  if (debug > 4)
    cout << "meta description: " << md << endl;
  current_meta_dsc = md;
}


//*****************************************************************************
// void TextCollector::got_meta_email(const char *e)
//
void
TextCollector::got_meta_email(const char *e)
{
  if (debug > 1)
    cout << "\nmeta email: " << e << endl;
  current_ref->DocEmail(e);
}


//*****************************************************************************
// void TextCollector::got_meta_notification(const char *e)
//
void
TextCollector::got_meta_notification(const char *e)
{
  if (debug > 1)
    cout << "\nmeta notification date: " << e << endl;
  current_ref->DocNotification(e);
}


//*****************************************************************************
// void TextCollector::got_meta_subject(const char *e)
//
void
TextCollector::got_meta_subject(const char *e)
{
  if (debug > 1)
    cout << "\nmeta subect: " << e << endl;
  current_ref->DocSubject(e);
}


//*****************************************************************************
// void TextCollector::got_noindex()
//
void
TextCollector::got_noindex()
{
  if (debug > 1)
    cout << "\nMETA ROBOT: Noindex " << current_ref->DocURL() << endl;
  current_ref->DocState(Reference_noindex);
}
