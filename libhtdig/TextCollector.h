//--------------------------------------------------------------------
//
// TextCollector.h
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
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: TextCollector.h,v 1.2 2003/06/23 22:28:17 nealr Exp $
//
//--------------------------------------------------------------------


#ifndef _TextCollector_h_
#define _TextCollector_h_

#include "BasicDocument.h"
#include "DocumentRef.h"
#include "Dictionary.h"
#include "Queue.h"
#include "HtWordReference.h"
#include "List.h"
#include "StringList.h"
#include "DocumentDB.h"

class Document;
class HtWordList;

enum  TextCollectorLog {
    TextCollector_noLog,
    TextCollector_logUrl,
    TextCollector_Restart
};

class TextCollector
{
    public:
        //
        // Construction/Destruction
        //
        			TextCollector(TextCollectorLog flags = TextCollector_noLog);
        virtual		~TextCollector();
    
        int        IndexDoc(BasicDocument & adoc);
        int        FlushWordDB();
    
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
        HtWordReference	word_context;
        HtWordList		words;
    	
        int			check_unique_md5;
        int			check_unique_date;
    
    
        TextCollectorLog log;
        //
        // These are weights for the words.  The index is the heading level.
        //
        long int		factor[11];
        int			currenthopcount;
    
        //
        // For efficiency reasons, we will only use one document object which
        // we reuse.
        //
        BasicDocument		*doc;
    
        Database 		*d_md5;
    
        // Some useful constants
        int              minimumWordLength;
    
        //
        // Helper routines
        //
        void		RetrievedDocument(DocumentRef *ref);
    
        int      temp_doc_count;
};

#endif


