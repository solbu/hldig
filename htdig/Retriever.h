//
// Retriever.h
//
// Retriever: Crawl from a list of URLs and calls appropriate parsers. The
//            parser notifies the Retriever object that it got something
//            (got_* functions) and the Retriever object feed the databases
//            and statistics accordingly.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Retriever.h,v 1.18.2.9 2000/10/10 03:15:36 ghutchis Exp $
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

class URL;
class Document;
class URLRef;
class HtWordList;

enum  RetrieverLog {
    Retriever_noLog,
    Retriever_logUrl,
    Retriever_Restart
};

class Retriever
{
public:
    //
    // Construction/Destruction
    //
    			Retriever(RetrieverLog flags = Retriever_noLog);
    virtual		~Retriever();

    //
    // Getting it all started
    //
    void		Initial(const String& url, int checked = 0);
    void		Initial(List &list , int checked = 0);
    void		Start();

    //
    // Report statistics about the parser
    //
    void		ReportStatistics(const String& name);
	
    //
    // These are the callbacks that we need to write code for
    //
    void		got_word(const char *word, int location, int heading);
    void		got_href(URL &url, const char *description, int hops = 1);
    void		got_title(const char *title);
    void		got_time(const char *time);
    void		got_head(const char *head);
    void		got_meta_dsc(const char *md);
    void		got_anchor(const char *anchor);
    void		got_image(const char *src);
    void		got_meta_email(const char *);
    void		got_meta_notification(const char *);
    void		got_meta_subject(const char *);
    void                got_noindex();

    //
    // Allow for the indexing of protected sites by using a
    // username/password
    //
    void		setUsernamePassword(const char *credentials);

    //
    // Routines for dealing with local filesystem access
    //
    StringList *	GetLocal(const String &strurl);
    StringList *	GetLocalUser(const String &url, StringList *defaultdocs);
    int			IsLocalURL(const String &url);

private:
    //
    // A hash to keep track of what we've seen
    //
    Dictionary		visited;
    
    URL			*base;
    String		current_title;
    String		current_head;
    String		current_meta_dsc;
    time_t		current_time;
    int			current_id;
    DocumentRef		*current_ref;
    int			current_anchor_number;
    int			trackWords;
    int			n_links;
    String		credentials;
    HtWordReference	word_context;
    HtWordList		words;
	
    int			check_unique_md5;
    int			check_unique_date;


    RetrieverLog log;
    //
    // These are weights for the words.  The index is the heading level.
    //
    long int		factor[10];
    int			currenthopcount;

    //
    // Some semi-constants...
    //
    int			max_hop_count;
	
    //
    // The list of server-specific information objects is indexed by
    // ip address and port number.  The list contains Server objects.
    //
    Dictionary		servers;

    //
    // For efficiency reasons, we will only use one document object which
    // we reuse.
    //
    Document		*doc;

    Database 		*d_md5;

    String		notFound;

    // Some useful constants
    int              minimumWordLength;

    //
    // Helper routines
    //
    int			Need2Get(const String &url);
    int			IsValidURL(const String &url);
    void		RetrievedDocument(Document &, const String &url, DocumentRef *ref);
    void		parse_url(URLRef &urlRef);
    void		got_redirect(const char *, DocumentRef *);
    void		recordNotFound(const String &url, const String &referer, int reason);
};

#endif


