//
// ResultList.cc
//
// Implementation of ResultList
//
//
#if RELEASE
static char RCSid[] = "$Id: ResultList.cc,v 1.4 1999/02/22 14:01:05 ghutchis Exp $";
#endif

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


