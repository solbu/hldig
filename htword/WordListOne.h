//
// WordListOne.h
//
// NAME
// 
// manage and use an inverted index file.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// WordContext context;
//
// WordList* words = context->List();
// WordList* words = WordListOne(&context);
// 
// DESCRIPTION
// 
// WordList is the <i>mifluz</i> equivalent of a database handler. Each
// WordList object is bound to an inverted index file and implements the
// operations to create it, fill it with word occurrences and search 
// for an entry matching a given criterion.
//
// The general behavious of WordListOne is described in the WordList
// manual page. It is prefered to create a WordListOne instance by
// setting the <i>wordlist_multi</i> configuration parameter to false
// and calling the <b>WordContext::List</b> method. 
// 
// Only the methods that differ from WordList are listed here.
// All the methods of WordList are implemented by WordListOne and
// you should refer to the manual page for more information.
//
// The <b>Cursor</b> methods all return a WordCursorOne instance
// cast to a WordCursor object.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordListOne.h,v 1.3 2003/06/24 19:57:27 nealr Exp $
//

#ifndef _WordListOne_h_
#define _WordListOne_h_

#include <fcntl.h>
#include <stdio.h>

#include "WordList.h"
#include "WordCursorOne.h"
#include "WordDict.h"
#include "WordMeta.h"
#include "WordDead.h"

class WordContext;

// 
// Inverted index interface
//
class WordListOne : public WordList
{
 public:
    //-
    // Constructor. Build inverted index handling object using
    // run time configuration parameters listed in the <b>CONFIGURATION</b>
    // section of the <b>WordList</b> manual page.
    //
    WordListOne(WordContext* ncontext);
    virtual ~WordListOne();

    virtual int Override(const WordReference& wordRef);

    virtual inline int Exists(const WordReference& wordRef) {
      return (!Dead()->Exists(wordRef.Key()) && db->Exists(wordRef) == 0) ? OK : NOTOK; }

    virtual int WalkDelete(const WordReference& wordRef);
    virtual inline int Delete(const WordReference& wordRef) {
      if(db->Del(wordRef) == 0)
	return dict->Unref(wordRef.GetWord());
      else
	return NOTOK;
    }
    //-
    // Delete the inverted index entry currently pointed to by the
    // <b>cursor.</b> 
    // Returns 0 on success, Berkeley DB error code on error. This
    // is mainly useful when implementing a callback function for
    // a <b>WordCursor.</b> 
    //
    int DeleteCursor(WordDBCursor& cursor) { return cursor.Del(); }

    virtual int Open(const String& filename, int mode);
    virtual int Close();
    virtual unsigned int Size() const;
    virtual int Pagesize() const {
      Configuration& config = context->GetConfiguration();

      return config.Value("wordlist_page_size", 0);
    }

    virtual inline WordDict *Dict() { return dict; }
    virtual inline WordMeta *Meta() { return meta; }
    virtual inline WordDead *Dead() { return dead; }

    virtual List *operator [] (const WordReference& wordRef);
    virtual List *Prefix (const WordReference& prefix);

    virtual List *Words() { return dict->Words(); }
    virtual List *WordRefs();

    virtual inline WordCursor *Cursor(wordlist_walk_callback_t callback, Object *callback_data) { return new WordCursorOne(this, callback, callback_data); }
    virtual inline WordCursor *Cursor(const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER) { return new WordCursorOne(this, searchKey, action); }
    virtual inline WordCursor *Cursor(const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data) { return new WordCursorOne(this, searchKey, callback, callback_data); }

    virtual WordKey Key(const String& bufferin);

    virtual WordReference Word(const String& bufferin, int exists = 0);

    virtual void BatchEnd();

    virtual int Noccurrence(const String& key, unsigned int& noccurrence) const;

    virtual int Write(FILE* f);

    virtual inline int WriteDict(FILE* f) { return dict->Write(f); }

    virtual int Read(FILE* f);

    virtual List *Collect(const WordReference& word);

    WordDB	            	*db;
    WordDict	            	*dict;
    WordMeta	            	*meta;
    WordDead	            	*dead;
};

#endif /* _WordListOne_h_ */

