//
// DocMatch.cc
//
// DocMatch: Data object only. Contains information related to a given
//           document that was matched by a search. For instance, the
//           score of the document for this search.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DocMatch.cc,v 1.3.2.1 2000/02/27 04:34:56 ghutchis Exp $
//

#include "DocMatch.h"


//*******************************************************************************
// DocMatch::DocMatch()
//
DocMatch::DocMatch()
{
    collection = NULL;
}


//*******************************************************************************
// DocMatch::~DocMatch()
//
DocMatch::~DocMatch()
{
}


