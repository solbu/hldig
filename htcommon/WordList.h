//
// WordList.h
//
// $Id: WordList.h,v 1.2 1997/03/24 04:33:15 turtle Exp $
//
// $Log: WordList.h,v $
// Revision 1.2  1997/03/24 04:33:15  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#ifndef _WordList_h_
#define _WordList_h_

#include <Dictionary.h>
#include <htString.h>


class WordList
{
public:
    //
    // Construction/Destruction
    //
    WordList();
    ~WordList();

    //
    // Set some operating parameters
    //
    void		WordTempFile(char *filename)	{tempfile = filename;}
    void		BadWordFile(char *filename);
    void		DocumentID(int id)		{docID = id;}

    //
    // Update/add a word
    //
    void		Word(char *word, int location, int anchor_number, double weight);

    //
    // Mark a document as already scanned for words or mark it as disappeared
    //
    void		MarkScanned();
    void		MarkGone();
    void		MarkModified();

    //
    // Dump the words to a temporary words file
    //
    void		Flush();

    //
    // Check if the given word is valid
    //
    int			IsValid(char *word)	{return valid_word(word);}

private:
    int			docID;
    String		tempfile;
    Dictionary		*words;
    Dictionary		badwords;

    int			valid_word(char *);
};

#endif


