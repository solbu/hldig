//
// Prefix.h
//
// $Id: Prefix.h,v 1.1 1998/06/21 23:20:04 turtle Exp $
//
// $Log: Prefix.h,v $
// Revision 1.1  1998/06/21 23:20:04  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Prefix_h_
#define _Prefix_h_

#include "Fuzzy.h"
#include "htfuzzy.h"

class Dictionary;
class String;
class List;


class Prefix : public Fuzzy
{
public:
    //
    // Construction/Destruction
    //
    Prefix();
    virtual		~Prefix();

    virtual void	getWords(char *word, List &words);
    virtual int		openIndex(Configuration &);

    virtual void	generateKey(char *, String &);
    virtual void	addWord(char *);
	
private:
};

#endif


