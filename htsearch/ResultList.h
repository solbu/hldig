//
// ResultList.h
//
// ResultList: A Dictionary indexed on the document id that holds
//             documents found for a search.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ResultList.h,v 1.4 1999/09/10 17:22:25 ghutchis Exp $
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


