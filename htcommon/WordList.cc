//
// WordList.cc
//
// Implementation of WordList
// Keeps track of the words in each document, spilling out to disk
// when the document is fully parsed
//
//
#if RELEASE
static char RCSid[] = "$Id: WordList.cc,v 1.16.2.5 2000/02/15 21:40:27 grdetil Exp $";
#endif

#include "WordList.h"
#include "WordReference.h"
#include "Configuration.h"
#include "htString.h"
#include <stdio.h>
#include <ctype.h>
#include "HtWordType.h"

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
  if (weight_factor == 0.0) // Why should we add words with no weight?
      return;
    String		shortword = word;
    static int	maximum_word_length = config.Value("maximum_word_length", 12);

    shortword.lowercase();
    word = shortword.get();
    if (shortword.length() > maximum_word_length)
	word[maximum_word_length] = '\0';

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
	wordRef->Word = word;
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
    static int	minimum_word_length = config.Value("minimum_word_length", 3);

    if (badwords.Exists(word))
	return 0;

    if (strlen(word) < minimum_word_length)
      return 0;

    while (word && *word)
    {
      if (HtIsStrictWordChar((unsigned char)*word) && !isdigit((unsigned char)*word))
	{
	    alpha = 1;
	    // break;	/* Can't stop here, there may still be control chars! */
	}
      else if (allow_numbers && isdigit((unsigned char)*word))
	{
	  alpha = 1;
	  // break;	/* Can't stop here, there may still be control chars! */
	}
//    if (*word >= 0 && *word < ' ')
      else if (iscntrl((unsigned char)*word))
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

    words->Start_Get();
    while ((wordRef = (WordReference *) words->Get_NextElement()))
    {

	fprintf(fl, "%s",wordRef->Word.get());
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
    static int	minimum_word_length = config.Value("minimum_word_length", 3);
    static int	maximum_word_length = config.Value("maximum_word_length", 12);

    while (fl && fgets(buffer, sizeof(buffer), fl))
    {
	word = strtok(buffer, "\r\n \t");
	if (word && *word)
	  {
	    if (strlen(word) > maximum_word_length)
	      word[maximum_word_length] = '\0';
	    new_word = word;  // We need to clean it up before we add it
	    new_word.lowercase();  // Just in case someone enters an odd one
	    HtStripPunctuation(new_word);
	    if (new_word.length() >= minimum_word_length)
	      badwords.Add(new_word, 0);
	  }
    }
    if (fl)
	fclose(fl);
}

