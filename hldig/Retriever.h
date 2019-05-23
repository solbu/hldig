//
// Retriever.h
//
// Retriever: Crawl from a list of URLs and calls appropriate parsers. The
//            parser notifies the Retriever object that it got something
//            (got_* functions) and the Retriever object feed the databases
//            and statistics accordingly.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Retriever.h,v 1.28 2004/05/28 13:15:15 lha Exp $
//

#ifndef _Retriever_h_
#define _Retriever_h_

#include "DocumentRef.h"
#include "Dictionary.h"
#include "Queue.h"
#include "HtWordReference.h"
#include "List.h"
#include "StringList.h"
#include "DocumentDB.h"

#define  HTDIG_ERROR_TESTURL_EXCLUDE           -109
#define  HTDIG_ERROR_TESTURL_BADQUERY          -110
#define  HTDIG_ERROR_TESTURL_EXTENSION         -111
#define  HTDIG_ERROR_TESTURL_EXTENSION2        -112
#define  HTDIG_ERROR_TESTURL_LIMITS            -113
#define  HTDIG_ERROR_TESTURL_LIMITSNORM        -114
#define  HTDIG_ERROR_TESTURL_SRCH_RESTRICT     -115
#define  HTDIG_ERROR_TESTURL_SRCH_EXCLUDE      -116
#define  HTDIG_ERROR_TESTURL_REWRITE_EMPTY     -117
#define  HTDIG_ERROR_TESTURL_ROBOT_FORBID      -118


class URL;
class Document;
class URLRef;
class HtWordList;

enum RetrieverLog
{
  Retriever_noLog,
  Retriever_logUrl,
  Retriever_Restart
};

struct word_entry:public Object
{
  word_entry (int loc, int fl, HtWordReference & ref):location (loc),
    flags (fl), context (ref)
  {
  };
  int location;
  int flags;
  HtWordReference context;
};

class Retriever
{
public:
  //
  // Construction/Destruction
  //
  Retriever (RetrieverLog flags = Retriever_noLog);
  virtual ~ Retriever ();

  //
  // Getting it all started
  //
  void Initial (const String & url, int checked = 0);
  void Initial (List & list, int checked = 0);
  void Start ();

  //
  // Report statistics about the parser
  //
  void ReportStatistics (const String & name);

  //
  // These are the callbacks that we need to write code for
  //
  void got_word (const char *word, int location, int heading);
  void got_href (URL & url, const char *description, int hops = 1);
  void got_title (const char *title);
  void got_author (const char *author);
  void got_time (const char *time);
  void got_head (const char *head);
  void got_meta_dsc (const char *md);
  void got_anchor (const char *anchor);
  void got_image (const char *src);
  void got_meta_email (const char *);
  void got_meta_notification (const char *);
  void got_meta_subject (const char *);
  void got_noindex ();

  //
  // Allow for the indexing of protected sites by using a
  // username/password
  //
  void setUsernamePassword (const char *credentials);

  //
  // Routines for dealing with local filesystem access
  //
  StringList *GetLocal (const String & strurl);
  StringList *GetLocalUser (const String & url, StringList * defaultdocs);
  int IsLocalURL (const String & url);

private:
  //
  // A hash to keep track of what we've seen
  //
    Dictionary visited;

  URL *base;
  String current_title;
  String current_head;
  String current_meta_dsc;
  time_t current_time;
  int current_id;
  DocumentRef *current_ref;
  int trackWords;
  int n_links;
  String credentials;
  HtWordReference word_context;
  HtWordList words;

  Dictionary words_to_add;

  int check_unique_md5;
  int check_unique_date;


  RetrieverLog log;
  //
  // These are weights for the words.  The index is the heading level.
  //
  long int factor[12];
  int currenthopcount;

  //
  // Some semi-constants...
  //
  int max_hop_count;

  //
  // The list of server-specific information objects is indexed by
  // ip address and port number.  The list contains Server objects.
  //
  Dictionary servers;

  //
  // For efficiency reasons, we will only use one document object which
  // we reuse.
  //
  Document *doc;

  Database *d_md5;

  String notFound;

  // Some useful constants
  int minimumWordLength;

  //
  // Helper routines
  //
  int Need2Get (const String & url);
  int IsValidURL (const String & url);
  void RetrievedDocument (Document &, const String & url, DocumentRef * ref);
  void parse_url (URLRef & urlRef);
  void got_redirect (const char *, DocumentRef *, const char * = 0);
  void recordNotFound (const String & url, const String & referer,
                       int reason);
};

#endif
