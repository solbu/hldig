//
// DocMatch.h
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
// $Id: DocMatch.h,v 1.4.2.1 2000/02/27 04:34:56 ghutchis Exp $
//

#ifndef _DocMatch_h_
#define _DocMatch_h_

#include "Object.h"

class Collection;

class DocMatch : public Object
{
public:
					DocMatch();
					~DocMatch();

	double				score;
	int				id;
	int				anchor;
	Collection                      *collection; // Multiple databases
};

#endif


