//
// ParseTree.h
//
// ParseTree: A parent class to parse user queries and turn
//            them into fully parsed trees of WeightWord objects
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ParseTree.h,v 1.1.2.2 2000/08/24 04:42:41 ghutchis Exp $
//

#ifndef _ParseTree_h_
#define _ParseTree_h_

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htString.h"
#include "List.h"
#include "StringList.h"
#include "WeightWord.h"
#include "ResultList.h"

#include <ctype.h>

class ParseTree: public Object
{
public:
    // Constructors
    ParseTree();
    ParseTree(String); // This is for creating a one-node tree of just this string

    // Destructor
    ~ParseTree();

    // Like the List and Stack and other container classes
    // Release disconnects the branches
    void		Release();
    // Destroy disconnects branches AND frees them
    void		Destroy();

    // Parse either a base string itself or a list of strings
    // Returns either OK or NOTOK as to the correctness of the query
    virtual int		Parse(String);
    virtual int		Parse(StringList *);

    // Gather result lists (including performing queries) and combine with
    // operator type (e.g. AND, OR, NOT, NEAR, etc.)
    virtual void	GetResults();
    
    // Adopt this ParseTree as one of our children
    // Alows us to build a tree through a bottom-up parse
    virtual void	Adopt(ParseTree *child);
    
    // If passed a list of Fuzzy methods, use them to fill out the tree
    // (note that some subclasses may choose to ignore this if desired)
    virtual void	Fuzzy(List);

    virtual String	GetName() { return "boolean"; }
    virtual String	GetLogicalWords();
    virtual String	GetQuery() { return initialQuery; }

protected:
    // This is really an n-ary tree
    List		*children;

    // This is intended mainly to serve as a cache key
    String		initialQuery;

    WeightWord		data;
    ResultList		results;

    String		WordToken(const String s, int &pointer);

};


#endif


