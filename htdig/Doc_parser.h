//
// Doc_Parser.h
//
// The Doc_Parser class is meant to be an interface which is
// implemented for each particular document type.
// Since C++ doesn't support interfaces, we'll have to settle for
// for an abstract class
//
// $Id: Doc_parser.h,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: Doc_parser.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _Doc_Parser_h_
#define _Doc_Parser_h_

#include "Retriever.h"

class Doc_Parser
{
public:
	//
	// Construction/Destruction
	//
					Doc_Parser();
	virtual			~Doc_Parser() = 0;

	virtual int		parse(Retriever &retriever) = 0;
};

#endif


