//
// AndParseTree.h
//
// AndParseTree: A class to parse queries of words to be merged together
//               to form an intersection (AND) of the matching documents.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: AndParseTree.h,v 1.1.2.1 2000/06/30 01:56:30 ghutchis Exp $
//

#ifndef _AndParseTree_h_
#define _AndParseTree_h_

#include "ParseTree.h"

class AndParseTree: public ParseTree
{
public:
    // Constructors
    AndParseTree();
    AndParseTree(String); // This is for creating a one-node tree of just this string

    // Destructor
    ~AndParseTree();

    // Parse either a base string itself or an inherited set
    // Returns either OK or NOTOK as to the correctness of the query
    virtual int		Parse(String);

    // Gather result lists (including performing queries) and combine with
    // operator type (e.g. AND, OR, NOT, NEAR, etc.)
    virtual void	GetResults();
    
    // If passed a list of Fuzzy methods, use them to fill out the tree
    // (note that some subclasses may choose to ignore this if desired)
    virtual void	Fuzzy(List);

    virtual String	GetName() { return "and"; }
    virtual String	GetLogicalWords();

private:
    // Everything is inherited from the parent class
};


#endif


