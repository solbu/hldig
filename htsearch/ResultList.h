//
// ResultList.h
//
// $Id: ResultList.h,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: ResultList.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _ResultList_h_
#define _ResultList_h_

#include <Dictionary.h>
#include "DocMatch.h"

class ResultList : public Dictionary
{
public:
    ResultList();
    ~ResultList();

    void		add(DocMatch *);
    void		remove(int id);
    DocMatch		*find(int id);
    DocMatch		*find(char *id);
    int			exists(int id);

    List		*elements();

    int			isIgnore;
};

#endif


