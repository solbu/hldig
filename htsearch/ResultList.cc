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
// $Id: ResultList.cc,v 1.6.2.2 2000/09/12 14:58:55 qss Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
    //Destroy();
}


//*****************************************************************************
//
void
ResultList::add(DocMatch *dm)
{
    String	t;
    t << dm->GetId();
    Add(t, dm);
}


//*****************************************************************************
//
DocMatch *
ResultList::find(int id) const
{
    String	t;
    t << id;
    return (DocMatch *) Find(t);
}


//*****************************************************************************
//
DocMatch *
ResultList::find(char *id) const
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
ResultList::exists(int id) const
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

void
ResultList::SetWeight(double weight)
{
	HtVector *els = elements();
	for(int i = 0; i < els->Count(); i++)
	{
		DocMatch *match = (DocMatch *)(*els)[i];
		match->SetWeight(weight);
	}
	els->Release();
}


ResultList::ResultList(const ResultList &other)
{
	DictionaryCursor c;
	isIgnore = other.isIgnore;
	other.Start_Get(c);
	DocMatch *match = (DocMatch *)other.Get_NextElement(c);
	while(match)
	{
		add(new DocMatch(*match));
		match = (DocMatch *)other.Get_NextElement(c);
	}
}

void
ResultList::Dump() const
{
	cerr << "ResultList {" << endl;
	cerr << "Ignore: " << isIgnore << " Count: " << Count() << endl;
	DictionaryCursor c;
	Start_Get(c);
	DocMatch *match = (DocMatch *)Get_NextElement(c);
	while(match)
	{
		match->Dump();
		match = (DocMatch *)Get_NextElement(c);
	}
	cerr << "}" << endl;
}
