//
// Retriever.h
//
// $Id: Retriever.h,v 1.8.2.4 2001/06/07 22:11:29 grdetil Exp $
//

#ifndef _Retriever_h_
#define _Retriever_h_

#include "DocumentRef.h"
#include "Images.h"
#include "Dictionary.h"
#include "Queue.h"
#include "List.h"
#include "StringList.h"

class URL;
class Document;
class URLRef;

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
    void		Initial(char *url, int checked = 0);
    void		Initial(List &list , int checked = 0);
    void		Start();

    //
    // Report statistics about the parser
    //
    void		ReportStatistics(char *name);
	
    //
    // These are the callbacks that we need to write code for
    //
    void		got_word(char *word, int location, int heading);
    void		got_href(URL &url, char *description);
    void		got_title(char *title);
    void		got_time(char *time);
    void		got_head(char *head);
    void		got_meta_dsc(char *md);
    void		got_anchor(char *anchor);
    void		got_image(char *src);
    void		got_meta_email(char *);
    void		got_meta_notification(char *);
    void		got_meta_subject(char *);
    void                got_noindex();

    //
    // Allow for the indexing of protected sites by using a
    // username/password
    //
	void		setUsernamePassword(char *credentials);

    //
    // Routines for dealing with local filesystem access
    //
    StringList *            GetLocal(char *url);
    StringList *            GetLocalUser(char *url, StringList *defaultdocs);
    int			IsLocalURL(char *url);
	
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
    Images		images;
    String		credentials;
	
    RetrieverLog log;
    //
    // These are weights for the words.  The index is the heading level.
    //
    double		factor[12];
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

    String		notFound;

    // Some useful constants
    int              minimumWordLength;

    //
    // Helper routines
    //
    int			Need2Get(char *url);
    DocumentRef	*	GetRef(char *url);
    int			IsValidURL(char *url);
    void		RetrievedDocument(Document &, char *url, DocumentRef *ref);
    void		parse_url(URLRef &urlRef);
    void		got_redirect(char *, DocumentRef *);
    void		recordNotFound(char *url, char *referer, int reason);
};

#endif


