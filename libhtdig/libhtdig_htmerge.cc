//----------------------------------------------------------------
//
// libhtdig_htmerge.cc
//
// 1/25/2002 created from htmerge.cc
//
// Neal Richter nealr@rightnow.com
//
// libhtdig_htmerge.cc
//
// htmerge: Merges two databases and/or updates databases to remove 
//          old documents and ensures the databases are consistent.
//          Calls db.cc, docs.cc, and/or words.cc as necessary
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_htmerge.cc,v 1.1 2003/04/09 00:50:36 nealr Exp $
//
//----------------------------------------------------------------

extern "C" {
#include "libhtdig_api.h"
}

#include "libhtdig_log.h"

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordContext.h"
#include "good_strtok.h"
#include "defaults.h"
#include "DocumentDB.h"
#include "HtURLCodec.h"
#include "HtWordList.h"
#include "HtWordReference.h"
#include "htString.h"

#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// If we have this, we probably want it.
//#ifdef HAVE_GETOPT_H
//#include <getopt.h>
//#endif





//Global Variables for this file

// This hash is used to keep track of all the document IDs which have to be
// discarded.
// This is generated from the doc database and is used to prune words
// from the word db
static Dictionary discard_list;

// This config is used for merging multiple databses
static HtConfiguration merge_config;
static HtConfiguration *config = NULL;

static int verbose = 0;
//static int stats = 0;
static int alt_work_area = 0;

//static String configFile = DEFAULT_CONFIG_FILE;
extern String configFile;

static String merge_configFile = 0;


// Component procedures
static int mergeDB ();

int  htmerge_index_merge(htmerge_parameters_struct *htmerge_parms)
{
    int ret = -1;
    int merge_ret = -1;

    //load htmerge 'command-line parameters'    
    configFile = htmerge_parms->configFile;
    merge_configFile = htmerge_parms->merge_configFile;
    verbose = htmerge_parms->debug;
    if(verbose != 0)
    {
        ret = logOpen(htmerge_parms->logFile);

        if(ret == FALSE)
        {
            reportError (form ("[HTDIG] Error opening log file [%s] . Error:[%d], %s\n",
                   htmerge_parms->logFile, errno, strerror(errno)) );
            return(HTMERGE_ERROR_LOGFILE_OPEN);
        }
    }
 
    alt_work_area = htmerge_parms->alt_work_area;



    config = HtConfiguration::config ();
    config->Defaults (&defaults[0]);

    if (access ((char *) configFile, R_OK) < 0)
    {
        reportError (form ("[HTMERGE] Unable to find configuration file '%s'",
                        configFile.get ()));
        return(HTMERGE_ERROR_CONFIG_READ);
    }

    config->Read (configFile);

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

    if (url_part_errors.length () != 0)
    {
        reportError (form("[HTMERGE] Invalid url_part_aliases or common_url_parts: %s",
                    url_part_errors.get ()));
        return(HTMERGE_ERROR_URL_PART);
    }

    if (merge_configFile.length ())
    {
        merge_config.Defaults (&defaults[0]);
        if (access ((char *) merge_configFile, R_OK) < 0)
        {
            reportError (form ("[HTMERGE] Unable to find configuration file '%s'",
                            merge_configFile.get ()));
            return(HTMERGE_ERROR_CONFIG_READ);
        }
        merge_config.Read (merge_configFile);
    }

    if (alt_work_area != 0)
    {
        String configValue;

        configValue = config->Find ("word_db");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("word_db", configValue);
        }

        configValue = config->Find ("doc_db");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_db", configValue);
        }

        configValue = config->Find ("doc_index");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_index", configValue);
        }

        configValue = config->Find ("doc_excerpt");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_excerpt", configValue);
        }
    }

    WordContext::Initialize(*config);

    if (merge_configFile.length())
    {
        // Merge the databases specified in merge_configFile into the current
        // databases. Do this first then update the other databases as usual
        // Note: We don't have to specify anything, it's all in the config vars

        merge_ret = mergeDB();
    }

    //call destructors here
    config->~HtConfiguration();
    merge_config.~HtConfiguration();

       if (verbose != 0)
    {
        ret = logClose();

        if (ret == FALSE)
        {
            reportError (form("[HTMERGE]: Error closing file [%s]. Error:[%d], %s\n",
                   htmerge_parms->logFile, errno, strerror(errno)) );
            return(HTMERGE_ERROR_LOGFILE_CLOSE);
        }
    }

    return(TRUE);
}

//*****************************************************************************
// void mergeDB()
//
static int mergeDB ()
{
    HtConfiguration *config = HtConfiguration::config ();
    DocumentDB merge_db, db;
    List *urls;
    Dictionary merge_dup_ids, db_dup_ids;    // Lists of DocIds to ignore
    int docIDOffset;

    const String doc_index = config->Find ("doc_index");
    if (access (doc_index, R_OK) < 0)
    {
        reportError (form
                   ("[HTMERGE] Unable to open document index '%s'",
                    (const char *) doc_index));
        return(HTMERGE_ERROR_DOCINDEX_READ);
    }
    const String doc_excerpt = config->Find ("doc_excerpt");
    if (access (doc_excerpt, R_OK) < 0)
    {
        reportError (form
                   ("[HTMERGE] Unable to open document excerpts '%s'",
                    (const char *) doc_excerpt));
        return(HTMERGE_ERROR_EXCERPTDB_READ);
    }
    const String doc_db = config->Find ("doc_db");
    if (db.Open (doc_db, doc_index, doc_excerpt) < 0)
    {
        reportError (form ("[HTMERGE] Unable to open/create document database '%s'",
                        (const char *) doc_db));
        return(HTMERGE_ERROR_DOCDB_READ);
    }


    const String merge_doc_index = merge_config["doc_index"];
    if (access (merge_doc_index, R_OK) < 0)
    {
        reportError (form
                   ("[HTMERGE] Unable to open document index '%s'",
                    (const char *) merge_doc_index));
        return(HTMERGE_ERROR_DOCINDEX_READ);
    }
    const String merge_doc_excerpt = merge_config["doc_excerpt"];
    if (access (merge_doc_excerpt, R_OK) < 0)
    {
        reportError (form
                   ("[HTMERGE] Unable to open document excerpts '%s'",
                    (const char *) merge_doc_excerpt));
        return(HTMERGE_ERROR_EXCERPTDB_READ);
    }
    const String merge_doc_db = merge_config["doc_db"];
    if (merge_db.Open (merge_doc_db, merge_doc_index, merge_doc_excerpt) < 0)
    {
        reportError (form ("[HTMERGE] Unable to open document database '%s'",
                        (const char *) merge_doc_db));
        return(HTMERGE_ERROR_DOCDB_READ);
    }

    // Start the merging by going through all the URLs that are in
    // the database to be merged

    urls = merge_db.URLs ();
    // This ensures that every document added from merge_db has a unique ID
    // in the new database
    docIDOffset = db.NextDocID ();

    urls->Start_Get ();
    String *url;
    String id;
    while ((url = (String *) urls->Get_Next ()))
    {
        DocumentRef *ref = merge_db[url->get ()];
        DocumentRef *old_ref = db[url->get ()];
        if (!ref)
            continue;

        if (old_ref)
        {
            // Oh well, we knew this would happen. Let's get the duplicate
            // And we'll only use the most recent date.

            if (old_ref->DocTime () >= ref->DocTime ())
            {
                // Cool, the ref we're merging is too old, just ignore it
                char str[20];
                sprintf (str, "%d", ref->DocID ());
                merge_dup_ids.Add (str, 0);

                if (verbose > 1)
                {
                    logEntry(form("[HTMERGE] Duplicate, URL: {%s} ignoring & merging copy\n", url));
                }
            }
            else
            {
                // The ref we're merging is newer, delete the old one and add
                char str[20];
                sprintf (str, "%d", old_ref->DocID ());
                db_dup_ids.Add (str, 0);
                db.Delete (old_ref->DocID ());
                ref->DocID (ref->DocID () + docIDOffset);
                db.Add (*ref);
                if (verbose > 1)
                {
                    logEntry(form("[HTMERGE] Duplicate, URL: {%s}  ignoring destination copy\n",url->get()));
                }
            }
        }
        else
        {
            // It's a new URL, just add it, making sure to load the excerpt
            merge_db.ReadExcerpt (*ref);
            ref->DocID (ref->DocID () + docIDOffset);
            db.Add (*ref);
            if (verbose > 1)
            {
                logEntry(form("[HTMERGE] Merged URL: {%s} \n",url->get()));
            }
        }
        delete ref;
        delete old_ref;
    }
    delete urls;

    // As reported by Roman Dimov, we must update db.NextDocID()
    // because of all the added records...
    db.IncNextDocID (merge_db.NextDocID ());
    merge_db.Close ();
    db.Close ();

    // OK, after merging the doc DBs, we do the same for the words
    HtWordList mergeWordDB (*config), wordDB (*config);
    List *words;
    String docIDKey;

    if (wordDB.Open (config->Find ("word_db"), O_RDWR) < 0)
    {
        reportError (form ("[HTMERGE] Unable to open/create word database '%s'",
                        (const char *) config->Find ("word_db")));
        return(HTMERGE_ERROR_WORDDB_READ);
    }

    if (mergeWordDB.Open (merge_config["word_db"], O_RDONLY) < 0)
    {
        reportError (form ("[HTMERGE] Unable to open word database '%s'",
                        (const char *) merge_config["word_db"]));
        return(HTMERGE_ERROR_WORDDB_READ);
    }

    // Start the merging by going through all the URLs that are in
    // the database to be merged

    words = mergeWordDB.WordRefs ();

    words->Start_Get ();
    HtWordReference *word;
    while ((word = (HtWordReference *) words->Get_Next ()))
    {
        docIDKey = word->DocID ();
        if (merge_dup_ids.Exists (docIDKey))
            continue;

        word->DocID (word->DocID () + docIDOffset);
        wordDB.Override (*word);
    }
    delete words;

    words = wordDB.WordRefs ();
    words->Start_Get ();
    while ((word = (HtWordReference *) words->Get_Next ()))
    {
        docIDKey = word->DocID ();
        if (db_dup_ids.Exists (docIDKey))
            wordDB.Delete (*word);
    }
    delete words;

    // Cleanup--just close the two word databases
    mergeWordDB.Close ();
    wordDB.Close ();

    return(TRUE);

}

