//
// ExactParseTree.h
//
// ExactParseTree: A class to parse phrase-match queries
//		   to form an intersection (and filtering) of results
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExactParseTree.h,v 1.1.2.1 2000/06/30 01:56:30 ghutchis Exp $
//

#ifndef _ExactParseTree_h_
#define _ExactParseTree_h_

#include "ParseTree.h"

#include <ctype.h>

class ExactParseTree: public ParseTree
{
public:
    // Constructors
    ExactParseTree();
    ExactParseTree(String); // This is for creating a one-node tree of just this string

    // Destructor
    ~ExactParseTree();

    // Parse either a base string itself or an inherited string
    // Returns either OK or NOTOK as to the correctness of the query
    virtual int		Parse(String);

    // Gather result lists (including performing queries) and combine with
    // operator type (e.g. AND, OR, NOT, NEAR, etc.)
    virtual void	GetResults();
    
    // If passed a list of Fuzzy methods, use them to fill out the tree
    // (note that some subclasses may choose to ignore this if desired)
    virtual void	Fuzzy(List);

    virtual String	GetName() { return "exact"; }
    virtual String	GetLogicalWords();

private:
    // Everything is inherited from the parent class
};


#endif


