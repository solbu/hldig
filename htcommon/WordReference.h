//
// WordReference.h
//
// WordReference: Reference to a word. Store everything we need for internal use
//                Defined as a class to allow the comparison 
//                method (for sorting).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.h,v 1.9 1999/09/28 14:35:37 loic Exp $
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include "htString.h"
#include "WordRecord.h"
#include "WordKey.h"
#include <stdio.h>

//
// Flags
// 
#define FLAG_TEXT 0
#define FLAG_CAPITAL 1
#define FLAG_TITLE 2
#define FLAG_HEADING 4
#define FLAG_KEYWORDS 8
#define FLAG_DESCRIPTION 16
#define FLAG_AUTHOR 32
#define FLAG_LINK_TEXT 64
#define FLAG_URL 128
// The remainder are undefined

class WordReference : public Object
{
public:
	//
	// Construction/Destruction
	//
        WordReference()	{ record.anchor = 0; }
        WordReference(const String& key, const String& record) {
	  Clear();
	  Unpack(key, record);
	}
        WordReference(const String& word) {
	  Clear();
	  Word(word);
	}
        WordReference(String word, unsigned int docid, unsigned int flags, unsigned int location, unsigned int anchor) {
	  Word(word);
	  DocID(docid);
	  Location(location);
	  Anchor(anchor);
	  Flags(flags);
	}

	~WordReference()	{}

	//
	// Accessors
	//
	String			Word() const { return key.GetWord(); }
	void			Word(const String& arg) { key.SetWord(arg); }
	unsigned int		DocID() const { return key.GetDocID(); }
	void			DocID(const unsigned int arg) { key.SetDocID(arg); }
	unsigned int		Flags() const { return key.GetFlags(); }
	void			Flags(const unsigned int arg) { key.SetFlags(arg); }
	unsigned int		Location() const { return key.GetLocation(); }
	void			Location(const unsigned int arg) { key.SetLocation(arg); }
	unsigned int		Anchor() const { return record.anchor; }
	void			Anchor(const unsigned int arg) { record.anchor = arg; }

	WordKey&		Key() { return key; }
	const WordKey&		Key() const { return key; }
	WordRecord&		Record() { return record; }
	const WordRecord&	Record() const { return record; }

	//
	// Conversion
	//
	void			Key(const WordKey& arg) { key = arg; }
	void			KeyUnpack(const String& packed) { key.Unpack(packed); }
	String			KeyPack() const { String tmp = 0; key.Pack(tmp); return tmp; }
	void			Record(const WordRecord& arg) { record = arg; }
	void			RecordUnpack(char* arg, int arg_length);

	int			Pack(String& ckey, String& crecord) const;
	int			Unpack(const String& ckey, const String& crecord);

	//
	// Mutations
	//
#define WORD_FILLED	1
#define WORD_PARTIAL	2
	int			Merge(const WordReference& other);
	static WordReference	Merge(const WordReference& master, const WordReference& slave) {
	  WordReference tmp(master);
	  tmp.Merge(slave);
	  return tmp;
	}

	void			Clear() { key.Clear(); record.anchor = 0; }
	int			compare(Object *to) { String word(((WordReference *) to)->Word()); return Word().nocase_compare(word); }

	//
	// Debuging
	//
	int			Dump(FILE *fl) const;
	static int		DumpHeader(FILE *fl);
	friend ostream		&operator << (ostream &o, const WordReference &wordRef);

private:

	WordKey			key;
	WordRecord		record;
};


#endif


