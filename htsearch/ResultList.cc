//
// ResultList.cc
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
// $Id: ResultList.cc,v 1.6 1999/09/10 17:22:25 ghutchis Exp $
//

#include "ResultList.h"
#include "htString.h"


//*****************************************************************************
// ResultList::ResultList()
//
ResultList::ResultList()
{
    isIgnore = 0;
}


//*****************************************************************************
// ResultList::~ResultList()
//
ResultList::~ResultList()
{
    Destroy();
}


//*****************************************************************************
//
void
ResultList::add(DocMatch *dm)
{
    String	t;
    t << dm->id;
    Add(t, dm);
}


//*****************************************************************************
//
DocMatch *
ResultList::find(int id)
{
    String	t;
    t << id;
    return (DocMatch *) Find(t);
}


//*****************************************************************************
//
DocMatch *
ResultList::find(char *id)
{
    return (DocMatch *) Find(id);
}


//*****************************************************************************
//
void
ResultList::remove(int id)
{
    String	t;
    t << id;
    Remove(t);
}


//*****************************************************************************
//
int
ResultList::exists(int id)
{
    String	t;
    t << id;
    return Exists(t);
}


//*****************************************************************************
//
HtVector *
ResultList::elements()
{
    HtVector	*list = new HtVector(Count() + 1);
    char	*id;

    Start_Get();
    while ((id = Get_Next()))
    {
	list->Add(Find(id));
    }
    return list;
}


