//
// Collection.h
//
// Collection: Specifies a list of databases to use in the search
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Collection.h,v 1.4 2004/05/28 13:15:24 lha Exp $
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

    void Open();

    void Close(); 

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
