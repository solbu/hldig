//
// HtWordReference.h
//
// HtWordReference: Reference to a word, derived from WordReference and
//		    implementing explicit accessors.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtWordReference.h,v 1.7 2004/07/11 10:28:22 lha Exp $
//
#ifndef _HtWordReference_h_
#define _HtWordReference_h_

#include "WordReference.h"
#include <stdio.h>

//
// Flags
// (If extra flags added, also update  htsearch.cc:colonPrefix)
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

// For field-restricted search, at least one of these flags must be set
// in document.  (255 = OR of the above...)
#define FLAGS_MATCH_ONE (255 | FLAG_PLAIN)

// The following are not stored in the database, but are used by WeightWord
#define FLAG_PLAIN 4096
#define FLAG_EXACT 8192
#define FLAG_HIDDEN 16384
#define FLAG_IGNORE 32768
// The remainder are undefined

class HtWordReference : public WordReference
{
public:
	//
	// Construction/Destruction
	//
        HtWordReference()	{}
        HtWordReference(const String& key, const String& record) :
	  WordReference(key, record) { }
        HtWordReference(const String& word) :
	  WordReference(word) {	}
        HtWordReference(String word, unsigned int docid, unsigned int flags, unsigned int location, unsigned int anchor) {
	  Word(word);
	  DocID(docid);
	  Location(location);
	  Anchor(anchor);
	  Flags(flags);
	}

	~HtWordReference()	{}

	//
	// Accessors
	//
	String			Word() const { return key.GetWord(); }
	void			Word(const String& arg) { key.SetWord(arg); }
	unsigned int		DocID() const { return key.Get( 1 ); }
	void			DocID(const unsigned int arg) { key.Set( 1, arg); }
	unsigned int		Flags() const { return key.Get( 2 ); }
	void			Flags(const unsigned int arg) { key.Set( 2, arg); }
	unsigned int		Location() const { return key.Get( 3 ); }
	void			Location(const unsigned int arg) { key.Set( 3, arg); }
	unsigned int		Anchor() const { return record.info.data; }
	void			Anchor(const unsigned int arg) { record.info.data = arg; }

	//
	// Dumping/Loading
	//
	int			Dump(FILE *fl) const;
	static int		DumpHeader(FILE *fl);
	int			Load(const String& s);
	static int		LoadHeader(FILE *fl);
};


#endif


