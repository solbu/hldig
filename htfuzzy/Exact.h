//
// Exact.h
//
// $Id: Exact.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Exact.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Exact_h_
#define _Exact_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Exact : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Exact();
    virtual		~Exact();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex(Configuration &);

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


