//----------------------------------------------------------------
//
// libhtdig_htfuzzy.cc
//
// 1/25/2002 created from htfuzzy.cc
//
// Neal Richter nealr@rightnow.com
//
// libhtdig_htfuzzy.cc
//
// htfuzzy: Create one or more ``fuzzy'' indexes into the main word database.
//          These indexes can be used by htsearch to perform a search that uses
//          other algorithms than exact word match.
//
//  This program is meant to be run after htmerge has created the word
//  database.
//
//  For each fuzzy algorithm, there will be a separate database.  Each
//  database is simply a mapping from the fuzzy key to a list of words
//  in the main word database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_htfuzzy.cc,v 1.4 2003/07/21 08:16:11 angusgb Exp $
//
//----------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

extern "C"
{
#include "libhtdig_api.h"
}

#include "libhtdig_log.h"


//#include "htfuzzy.h"  //NOT USED

#include "Fuzzy.h"
#include "Accents.h"
#include "Soundex.h"
#include "Endings.h"
#include "Metaphone.h"
#include "Synonym.h"
#include "htString.h"
#include "List.h"
#include "Dictionary.h"
#include "defaults.h"
#include "HtWordList.h"
#include "WordContext.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "HtConfiguration.h"
#include "HtWordList.h"

#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

#include <stdio.h>



extern int debug;

static HtConfiguration * config = NULL;


//*****************************************************************************
// int main(int ac, char **av)
//
//int main(int ac, char **av)

int htfuzzy_index(htfuzzy_parameters_struct * htfuzzy_parms)
{
    String configFile = DEFAULT_CONFIG_FILE;
    int ret = 0;

    //
    // Parse command line arguments
    //

    debug = htfuzzy_parms->debug;
    if (debug != 0)
    {
        ret = logOpen(htfuzzy_parms->logFile);

        if (ret == FALSE)
        {
            fprintf(stderr, "htdig: Error opening file [%s]. Error:[%d], %s\n",
                   htfuzzy_parms->logFile, errno, strerror(errno));
        }
    }


    configFile = htfuzzy_parms->configFile;

    config = HtConfiguration::config();
    
    //
    // Determine what algorithms to use
    //
    List wordAlgorithms;
    List noWordAlgorithms;

    if (htfuzzy_parms->algorithms_flag & HTDIG_ALG_SOUNDEX)
    {
        wordAlgorithms.Add(new Soundex(*config));
    }
    else if (htfuzzy_parms->algorithms_flag & HTDIG_ALG_METAPHONE)
    {
        wordAlgorithms.Add(new Metaphone(*config));
    }
    else if (htfuzzy_parms->algorithms_flag & HTDIG_ALG_ACCENTS)
    {
        wordAlgorithms.Add(new Accents(*config));
    }
    else if (htfuzzy_parms->algorithms_flag & HTDIG_ALG_ENDINGS)
    {
        noWordAlgorithms.Add(new Endings(*config));
    }
    else if (htfuzzy_parms->algorithms_flag & HTDIG_ALG_SYNONYMS)
    {
        noWordAlgorithms.Add(new Synonym(*config));
    }


    if (wordAlgorithms.Count() == 0 && noWordAlgorithms.Count() == 0)
    {
        logEntry(form("htfuzzy: No algorithms specified\n"));
    }

    //
    // Find and parse the configuration file.
    //
    config->Defaults(&defaults[0]);
    if (access((char *) configFile, R_OK) < 0)
    {
        reportError(form("[HTFUZZY] Unable to find configuration file '%s'", configFile.get()));
    }
    config->Read(configFile);

    // Initialize htword library (key description + wordtype...)
    WordContext::Initialize(*config);

    Fuzzy *fuzzy;
    if (wordAlgorithms.Count() > 0)
    {
        //
        // Open the word database so that we can grab the words from it.
        //
        HtWordList worddb(*config);
        if (worddb.Open(config->Find("word_db"), O_RDONLY) == OK)
        {
            //
            // Go through all the words in the database
            //
            List *words = worddb.Words();
            String *key;
            Fuzzy *fuzzy = 0;
            String word, fuzzyKey;
            int count = 0;

            words->Start_Get();
            while ((key = (String *) words->Get_Next()))
            {
                word = *key;
                wordAlgorithms.Start_Get();
                while ((fuzzy = (Fuzzy *) wordAlgorithms.Get_Next()))
                {
                    fuzzy->addWord(word);
                }
                count++;
                if ((count % 100) == 0 && debug)
                {
                    //cout << "htfuzzy: words: " << count << '\n';
                }
            }
            if (debug)
            {
                logEntry(form("htfuzzy: total words: %d\n", count));
                logEntry(form("htfuzzy: Writing index files...\n"));
            }

            //
            // All the information is now in memory.
            // Write all of it out to the individual databases
            //
            wordAlgorithms.Start_Get();
            while ((fuzzy = (Fuzzy *) wordAlgorithms.Get_Next()))
            {
                fuzzy->writeDB();
            }
            worddb.Close();
            words->Destroy();
            delete words;
            if (fuzzy)
                delete fuzzy;
        }
        else
        {
            reportError(form("[htfuzzy] Unable to open word database %s", config->Find("word_db").get()));
        }
    }
    if (noWordAlgorithms.Count() > 0)
    {
        noWordAlgorithms.Start_Get();
        while ((fuzzy = (Fuzzy *) noWordAlgorithms.Get_Next()))
        {
            if (debug)
            {
                logEntry(form( "htfuzzy: Selected algorithm: %s\n", fuzzy->getName()));
            }
            if (fuzzy->createDB(*config) == NOTOK)
            {
                logEntry(form("htfuzzy: Could not create database for algorithm: %s\n", fuzzy->getName()));
            }
        }
    }

    if (debug)
    {
        logEntry("htfuzzy: Done.\n");
    }

       if (debug != 0)
    {
        ret = logClose();

        if (ret == FALSE)
        {
            fprintf(stderr, "htfuzzy: Error closing file [%s]. Error:[%d], %s\n",
                   htfuzzy_parms->logFile, errno, strerror(errno));
        }
    }


    delete config;

    return 0;
}


