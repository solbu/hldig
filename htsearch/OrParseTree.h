//
// OrParseTree.h
//
// OrParseTree: A class to parse queries of words to be merged together
//              to form a union (OR) of the matching documents.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: OrParseTree.h,v 1.1.2.1 2000/06/30 01:56:30 ghutchis Exp $
//

#ifndef _OrParseTree_h_
#define _OrParseTree_h_

#include "ParseTree.h"

#include <ctype.h>

class OrParseTree: public ParseTree
{
public:
    // Constructors
    OrParseTree();
    OrParseTree(String); // This is for creating a one-node tree of just this string

    // Destructor
    ~OrParseTree();

    // Parse either a base string itself or an inherited string
    // Returns either OK or NOTOK as to the correctness of the query
    virtual int		Parse(String);

    // Gather result lists (including performing queries) and combine with
    // operator type (e.g. AND, OR, NOT, NEAR, etc.)
    virtual void	GetResults();
    
    // If passed a list of Fuzzy methods, use them to fill out the tree
    // (note that some subclasses may choose to ignore this if desired)
    virtual void	Fuzzy(List);

    virtual String	GetName() { return "or"; }
    virtual String	GetLogicalWords();

private:
    // Everything is inherited from the parent class
};


#endif


