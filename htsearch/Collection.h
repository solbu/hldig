//
// Collection.h
//
// Collection: Specifies a list of databases to use in the search
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Collection.h,v 1.1.2.2 2000/10/20 03:40:57 ghutchis Exp $
//
#ifndef _Collection_h_
#define _Collection_h_

#include "Object.h"
#include "ResultList.h"
#include "ResultMatch.h"
#include "TemplateList.h"
#include "cgi.h"
#include "StringMatch.h"
#include "List.h"
#include "DocumentDB.h"
#include "Database.h"
#include "Dictionary.h"

class Collection : public Object
{
public:
    //
    // Construction/Destruction
    //
    Collection(const char *name, const char *wordFile, 
               const char *indexFile, const char *docFile,
               const char *docExcerpt);
    ~Collection();

    void Collection::Open();

    void Collection::Close(); 

    char *getWordFile() { return wordFile.get(); }
    DocumentRef         *getDocumentRef(int id);
    ResultList		*getResultList() { return matches; }
    void		setResultList(ResultList *list) { matches = list; }

    List                *getSearchWords() { return searchWords; }
    void                setSearchWords(List *list) { searchWords = list; }

    StringMatch         *getSearchWordsPattern() { return searchWordsPattern;}
    void                setSearchWordsPattern(StringMatch *smatch)
                            { searchWordsPattern = smatch; }
                  
    int			ReadExcerpt(DocumentRef &ref);

protected:
    String              collectionName;
    String              wordFile;
    String              indexFile;
    String              docFile;
    String		docExcerpt;
    ResultList		*matches;
    List                *searchWords;
    StringMatch         *searchWordsPattern;

    DocumentDB          docDB;
    // Database         *docIndex;     

    int                 isopen;
};

#endif // _Collection_h_
