//
// WordSearcher.cc
//
// WordSearcher: a simple word database readonly-access wrapper
//               generates ResultLists for the Query framework.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordSearcher.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#include "WordSearcher.h"
#include "WordType.h"
#include "ResultList.h"
#include "HtWordReference.h"
#include "defaults.h"

extern int debug;

//
// constructor, opens the database
//
WordSearcher::WordSearcher(const String &filename) :
	references(*(HtConfiguration::config()))
{
	references.Open(filename, O_RDONLY);
}

//
// gather results for a word, either from db or ignored
//
ResultList *
WordSearcher::Search(const String &word)
{
	ResultList *result = 0;
	if(IsIgnore(word))
	{
		if(debug) cerr << "IGNORE: " << word << endl;
		result = new ResultList;
		result->Ignore();
	}
	else
	{
		result = Fetch(word);
	}
	return result;
}

//
// see if word must be ignored
//
bool
WordSearcher::IsIgnore(const String &word)
{
	HtConfiguration* config= HtConfiguration::config();
	String copy = word;
	WordType type(*config);
	return 0 != type.Normalize(copy);
}

//
// gather all references in the db, construct a ResultList
//
ResultList *
WordSearcher::Fetch(const String &word)
{
	if(debug) cerr << "FETCH: " << word << endl;
	ResultList *result = 0;
	List *refs = references[word];

	if(refs && refs->Count())
	{
		if(debug) cerr << "REFERENCES: " << refs->Count() << endl;
		result = new ResultList;
		DocMatch *match = new DocMatch;

		refs->Start_Get();
		HtWordReference *ref = (HtWordReference *)refs->Get_Next();
		match->SetId(ref->DocID());
		match->SetAnchor(ref->Anchor());
		result->add(match);
		unsigned int current = ref->DocID();
		if(debug) cerr << "At: " << ref->DocID() << endl;
		while(ref)
		{
			if(ref->DocID() != current)
			{
				if(debug) cerr << "At: "<<ref->DocID()<< endl;
				match = new DocMatch;
				match->SetId(ref->DocID());
				match->SetAnchor(ref->Anchor());
				result->add(match);
				current = ref->DocID();
			}
			if(debug) cerr << "@ "<<ref->Location()<< endl;
			match->AddLocation(
				new Location(
					ref->Location(),
					ref->Location(),
					ref->Flags()));
			ref = (HtWordReference *)refs->Get_Next();
		}
	}
	return result;
}
