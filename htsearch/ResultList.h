//
// ResultList.h
//
// $Id: ResultList.h,v 1.2 1999/02/22 14:01:05 ghutchis Exp $
//
//
#ifndef _ResultList_h_
#define _ResultList_h_

#include "Dictionary.h"
#include "DocMatch.h"
#include "HtVector.h"

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

    HtVector		*elements();

    int			isIgnore;
};

#endif


