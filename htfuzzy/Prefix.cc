//
// Prefix.cc
//
// Implementation of Prefix
//
// $Log: Prefix.cc,v $
// Revision 1.3  1998/09/30 17:31:51  ghutchis
//
// Changes for 3.1.0b2
//
// Revision 1.2  1998/08/03 16:50:38  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.1  1998/06/21 23:20:04  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.2  1997/03/24 04:33:18  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Prefix.cc,v 1.3 1998/09/30 17:31:51 ghutchis Exp $";
#endif

#include "Prefix.h"
#include <htString.h>
#include <List.h>
#include <StringMatch.h>
#include <Configuration.h>

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

    Database	*dbf = Database::getDatabaseInstance();
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
    String w3 = new String(w2);
    w3.lowercase();
    dbf->Start_Seq(w3.get());

    while (wordCount < maximumWords && (s = dbf->Get_Next_Seq()))
    {
	if (strncasecmp(s, w, len))
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




