//
// ResultList.cc
//
// Implementation of ResultList
//
// $Log: ResultList.cc,v $
// Revision 1.3  1998/08/03 16:50:46  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.2  1997/03/24 04:33:24  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: ResultList.cc,v 1.3 1998/08/03 16:50:46 ghutchis Exp $";
#endif

#include "ResultList.h"
#include <htString.h>


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
List *
ResultList::elements()
{
    List	*list = new List;
    char	*id;

    Start_Get();
    while ((id = Get_Next()))
    {
	list->Add(Find(id));
    }
    return list;
}


