//
// ResultList.h
//
// ResultList: A Dictionary indexed on the document id that holds
//             documents found for a search.
//
// $Id: ResultList.h,v 1.3 1999/09/09 10:16:07 loic Exp $
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


