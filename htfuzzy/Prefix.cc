//
// Prefix.cc
//
// Implementation of Prefix
//
// $Id: Prefix.cc,v 1.7 1999/03/03 04:46:57 ghutchis Exp $
//
#if RELEASE
static char RCSid[] = "$Id: Prefix.cc,v 1.7 1999/03/03 04:46:57 ghutchis Exp $";
#endif

#include "Prefix.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

extern Configuration	config;


//*****************************************************************************
// Prefix::Prefix()
//
Prefix::Prefix()
{
}


//*****************************************************************************
// Prefix::~Prefix()
//
Prefix::~Prefix()
{
}


//*****************************************************************************
//
//  Prefix search
//
void
Prefix::getWords(char *w, List &words)
{

    if (w == NULL || w[0] == '\0')
	return;

    char 	*prefix_suffix = config["prefix_match_character"];
    int 	prefix_suffix_length = prefix_suffix == NULL 
					? 0 : strlen(prefix_suffix);
    int 	minimum_prefix_length = config.Value("minimum_prefix_length");

    if (debug)
         cerr << " word=" << w << " prefix_suffix=" << prefix_suffix 
		<< " prefix_suffix_length=" << prefix_suffix_length
		<< " minimum_prefix_length=" << minimum_prefix_length << "\n";

    if (strlen(w) < minimum_prefix_length + prefix_suffix_length)
	return;

    //  A null prefix character means that prefix matching should be 
    //  applied to every search word; otherwise return if the word does 
    //	not end in the prefix character(s).
    //
    if (prefix_suffix_length > 0
	    && strcmp(prefix_suffix, w+strlen(w)-prefix_suffix_length)) 
	return;

    Database	*dbf = Database::getDatabaseInstance(DB_BTREE);
    dbf->OpenRead(config["word_db"]);

    int		wordCount = 0;
    int		maximumWords = config.Value("max_prefix_matches", 1000);
    char	*s;
    int		len = strlen(w) - prefix_suffix_length;
    
    // Strip the prefix character(s)
    char w2[8192];
    strncpy(w2, w, sizeof(w2) - 1);
    w2[sizeof(w2) - 1] = '\0';
    w2[strlen(w2) - prefix_suffix_length] = '\0';
    String w3(w2);
    w3.lowercase();
    dbf->Start_Seq(w3.get());

    while (wordCount < maximumWords && (s = dbf->Get_Next_Seq()))
    {
	if (mystrncasecmp(s, w, len))
	    break;
	words.Add(new String(s));
	wordCount++;
    }
    dbf->Close();
    delete dbf;
}


//*****************************************************************************
int
Prefix::openIndex(Configuration &)
{
  return 0;
}


//*****************************************************************************
void
Prefix::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Prefix::addWord(char *)
{
}




