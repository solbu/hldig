//
// WordList.cc
//
// Implementation of WordList
//
// $Log: WordList.cc,v $
// Revision 1.12  1999/01/14 00:28:14  ghutchis
// Changed field order in db.wordlist. With the old order, words from HTML body
// and words from links to that url weren't merged sometimes.
//
// Revision 1.11  1999/01/10 02:00:17  ghutchis
// Break out of looping once we're sure the word is invalid.
//
// Revision 1.10  1998/12/13 06:13:13  ghutchis
// Change undefined minimumWordLength to config("minimum_word_length").
//
// Revision 1.9  1998/12/13 00:15:35  ghutchis
// Added additional cleanups for words in the bad word list. Check to make sure
// they don't have punctuation, etc.
//
// Revision 1.8  1998/12/08 02:52:19  ghutchis
// Remove unnecessary code.
//
// Revision 1.7  1998/12/06 18:45:57  ghutchis
// Ensure duplicate words have minimum location and anchor attributes.
//
// Revision 1.6  1998/12/05 00:53:23  ghutchis
// Don't store c:1 and a:0 entries in db.wordlist to save space.
//
// Revision 1.5  1998/11/27 18:30:20  ghutchis
// Fixed bug with bad_words and MAX_WORD_LENGTH, noted by Jeff Breidenbach
// <jeff@alum.mit.edu>.
//
// Revision 1.4  1998/09/04 00:56:22  ghutchis
// Various bug fixes.
//
// Revision 1.3  1997/03/24 04:33:15  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.2  1997/02/24 17:52:39  turtle
// Applied patches supplied by "Jan P. Sorensen" <japs@garm.adm.ku.dk> to make
// ht://Dig run on 8-bit text without the global unsigned-char option to gcc.
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: WordList.cc,v 1.12 1999/01/14 00:28:14 ghutchis Exp $";
#endif

#include "WordList.h"
#include <WordReference.h>
#include <Configuration.h>
#include <htString.h>
#include <stdio.h>
#include <ctype.h>

extern Configuration	config;

//*****************************************************************************
// WordList::WordList()
//
WordList::WordList()
{
    words = new Dictionary(203, 10.0);
}


//*****************************************************************************
// WordList::~WordList()
//
WordList::~WordList()
{
    delete words;
}


//*****************************************************************************
// void WordList::Word(char *word, int location, int anchor_number, double weight_factor)
//
void WordList::Word(char *word, int location, int anchor_number, double weight_factor)
{
    String		shortword = word;

    shortword.lowercase();
    word = shortword.get();
    if (shortword.length() > MAX_WORD_LENGTH)
	word[MAX_WORD_LENGTH] = '\0';

    if (!valid_word(word))
	return;

    WordReference	*wordRef = (WordReference *) words->Find(word);

    if (wordRef)
    {
	//
	// Already had this word.  Update the record
	//
	wordRef->WordCount++;
	wordRef->Weight += int((1000 - location) * weight_factor);
	if (location < wordRef->Location)
	  wordRef->Location = location;
	if (anchor_number < wordRef->Anchor)
	  wordRef->Anchor = anchor_number;
    }
    else
    {
	//
	// New word.  Create a new reference for it
	//
	wordRef = new WordReference;
	wordRef->WordCount = 1;
	wordRef->Location = location;
	wordRef->DocumentID = docID;
	wordRef->Weight = int((1000 - location) * weight_factor);
	wordRef->Anchor = anchor_number;
	strcpy(wordRef->Word, word);
	words->Add(word, wordRef);
    }
}


//*****************************************************************************
// int WordList::valid_word(char *word)
//   Words are considered valid if they contain alpha characters and
//   no control characters.  Also, if the word is found in the list of
//   bad words, it will be marked invalid.
//
int WordList::valid_word(char *word)
{
    int		control = 0;
    int		alpha = 0;
    static int	allow_numbers = config.Boolean("allow_numbers", 0);

    if (badwords.Exists(word))
	return 0;

    while (word && *word)
    {
      if (isalpha((unsigned char)*word))
	{
	    alpha = 1;
	    break;
	}
      if (allow_numbers && isdigit(*word))
	{
	  alpha = 1;
	  break;
	}
      if (*word >= 0 && *word < ' ')
	{
	    control = 1;
	    break;
	}
      word++;
    }

    return alpha && !control;
}


//*****************************************************************************
// void WordList::Flush()
//   Dump the current list of words to the temporary word file.  After
//   the words have been dumped, the list will be destroyed to make
//   room for the words of the next document.
//   
void WordList::Flush()
{
    FILE		*fl = fopen(tempfile, "a");
    WordReference	*wordRef;
    char		*word;

    words->Start_Get();
    while ((word = words->Get_Next()))
    {
	wordRef = (WordReference *) words->Find(word);

	fprintf(fl, "%s",wordRef->Word);
        fprintf(fl, "\ti:%d\tl:%d\tw:%d",
		wordRef->DocumentID,
	        wordRef->Location,
		wordRef->Weight);
        if (wordRef->WordCount != 1)
        {
           fprintf(fl, "\tc:%d",wordRef->WordCount);
        }
	if (wordRef->Anchor != 0)
        {
           fprintf(fl, "\ta:%d",wordRef->Anchor);
        }
        putc('\n', fl);
    }
    words->Destroy();
    fclose(fl);
}


//*****************************************************************************
// void WordList::MarkScanned()
//   The current document hasn't changed.  No need to redo the words.
//
void WordList::MarkScanned()
{
    FILE			*fl = fopen(tempfile, "a");

    fprintf(fl, "+%d\n", docID);
    words->Destroy();
    fclose(fl);
}


//*****************************************************************************
// void WordList::MarkGone()
//   The current document has disappeared.  The merger can take out
//   all references to the current doc.
//
void WordList::MarkGone()
{
    FILE			*fl = fopen(tempfile, "a");

    fprintf(fl, "-%d\n", docID);
    words->Destroy();
    fclose(fl);
}


//*****************************************************************************
// void WordList::MarkModified()
//   The current document has disappeared.  The merger can take out
//   all references to the current doc.
//
void WordList::MarkModified()
{
    FILE			*fl = fopen(tempfile, "a");

    fprintf(fl, "!%d\n", docID);
    words->Destroy();
    fclose(fl);
}


//*****************************************************************************
// void WordList::BadWordFile(char *filename)
//   Read in a list of words which are not to be included in the word
//   file.  This is mostly to exclude words that are too common.
//
void WordList::BadWordFile(char *filename)
{
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];
    char	*word;
    String      new_word;
    char        *valid_punctuation = config["valid_punctuation"];

    while (fl && fgets(buffer, sizeof(buffer), fl))
    {
	word = strtok(buffer, "\r\n \t");
	if (word && *word)
	  {
	    if (strlen(word) > MAX_WORD_LENGTH)
	      word[MAX_WORD_LENGTH] = '\0';
	    new_word = word;  // We need to clean it up before we add it
	    new_word.lowercase();  // Just in case someone enters an odd one
	    new_word.remove(valid_punctuation);
	    if (new_word.length() >= config.Value("minimum_word_length", 3))
	      badwords.Add(new_word, 0);
	  }
    }
    if (fl)
	fclose(fl);
}

