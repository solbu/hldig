//
// HtWordReference.h
//
// HtWordReference: Reference to a word, derived from WordReference and
//		    implementing explicit accessors.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordReference.h,v 1.1 1999/10/01 15:19:28 loic Exp $
//
#ifndef _HtWordReference_h_
#define _HtWordReference_h_

#include "WordReference.h"
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
	unsigned int		DocID() const { return key.GetDocID(); }
	void			DocID(const unsigned int arg) { key.SetDocID(arg); }
	unsigned int		Flags() const { return key.GetFlags(); }
	void			Flags(const unsigned int arg) { key.SetFlags(arg); }
	unsigned int		Location() const { return key.GetLocation(); }
	void			Location(const unsigned int arg) { key.SetLocation(arg); }
	unsigned int		Anchor() const { return record.info.data; }
	void			Anchor(const unsigned int arg) { record.info.data = arg; }

	//
	// Debuging
	//
	int			Dump(FILE *fl) const;
	static int		DumpHeader(FILE *fl);
};


#endif


