//
// Substring.h
//
// $Id: Substring.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Substring.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Substring_h_
#define _Substring_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Substring : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Substring();
    virtual		~Substring();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex(Configuration &);

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


