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
// $Id: DocMatch.cc,v 1.3.2.2 2000/05/06 20:46:41 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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


