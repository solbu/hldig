#ifndef _OrQueryParser_h_
#define _OrQueryParser_h_

//
// OrQueryParser.h
//
// OrQueryParser: a query parser for 'any word' (or) queries
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: OrQueryParser.h,v 1.3 2003/06/24 19:58:07 nealr Exp $
//

#include "SimpleQueryParser.h"
#include "OrQuery.h"

class OrQueryParser : public SimpleQueryParser
{
public:
	OrQueryParser() {}

private:
	OperatorQuery *MakeQuery()
	{
		return new OrQuery;
	}
};

#endif
