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
// $Id: ResultList.h,v 1.5 1999/10/15 03:30:41 jtillman Exp $
//

#ifndef _ResultList_h_
#define _ResultList_h_

#include "Dictionary.h"
#include "DocMatch.h"
#include "HtVector.h"
#include "DocumentDB.h"

class ResultList : public Dictionary
{
public:
	ResultList(const String& docFile, const String&, const String&);
  ResultList();
	~ResultList();

	void add(DocMatch *);
  void remove(int id);
  DocMatch *find(int id);
  DocMatch *find(char *id);
  int exists(int id);

  HtVector *elements();

	/** Returns a reference to the data for the
		document matching the id provided */
  DocumentRef *getDocumentRef(int docID);
  /** Retrieves the excerpt for the document into 
memory */
  void readExcerpt(DocumentRef &ref);

	int			isIgnore;

protected: // Protected attributes
  /** Stores a reference to the database where 
the data for each match can be retrieved */
  DocumentDB docDB;
  /** Indicates whether this is a "smart" ResultList
or one that does not have a database
connection */
  int dbIsOpen;
};

#endif













