//
// Endings.h
//
// $Id: Endings.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Endings.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Endings_h_
#define _Endings_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Endings : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Endings();
    virtual		~Endings();

    virtual void	getWords(char *word, List &words);
    virtual void	generateKey(char *word, String &key);
    virtual void	addWord(char *word);
    virtual int		openIndex(Configuration &);
    virtual int		writeDB(Configuration &);

    //
    // Special member which will create the two databases needed for this
    // algorithm.
    //
    int			createDB(Configuration &config);
	
    static void		mungeWord(char *, String &);
    
private:
    Database		*root2word;
    Database		*word2root;

    int			createRoot(Dictionary &, char *, char *, char *);
    int			readRules(Dictionary &, char *);
    void		expandWord(String &, List &, Dictionary &, char *, char *);
};

#endif


