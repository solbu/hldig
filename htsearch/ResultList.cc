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
// $Id: ResultList.cc,v 1.7 1999/10/15 03:31:13 jtillman Exp $
//

#include "ResultList.h"
#include "htString.h"
#include "defaults.h"

//*****************************************************************************
// ResultList::ResultList()
//
ResultList::ResultList(const String& docFile, const String& indexFile, const String& excerptFile)
{

//This provides for a smarter "db-aware" result list, which can
// retrieve the data for its matches
    docDB.SetCompatibility(config.Boolean("uncoded_db_compatible", 1));
    docDB.Read(docFile, indexFile, excerptFile);
    dbIsOpen = 1;
    isIgnore = 0;
}

ResultList::ResultList()
{
		//This constructor creates the original "non-db-aware" ResultList
		dbIsOpen = 0;
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




/** Returns a reference to the data for the 
document matching the id provided */
DocumentRef *ResultList::getDocumentRef(int docID){
	if (dbIsOpen) {
		return docDB[docID];
	}
	else
		return 0;
}


/** Retrieves the excerpt for the document into 
memory */
void ResultList::readExcerpt(DocumentRef &ref)
{
	if (dbIsOpen)
		docDB.ReadExcerpt(ref);
}

