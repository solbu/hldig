//
// Synonym.h
//
// $Id: Synonym.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Synonym.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Synonym_h_
#define _Synonym_h_

#include "Fuzzy.h"

class List;

class Synonym : public Fuzzy
{
public:
	//
	// Construction/Destruction
	//
					Synonym();
					~Synonym();

	//
	// Lookup routines
	//
	virtual void	getWords(char *word, List &words);
	virtual int		openIndex(Configuration &);

	//
	// Creation
	//
	virtual int		createDB(Configuration &config);
	
protected:

	Database		*db;
};

#endif


