//
// BTree.h
//
// $Id: BTree.h,v 1.2 1999/01/23 01:25:02 hp Exp $
//
// $Log: BTree.h,v $
// Revision 1.2  1999/01/23 01:25:02  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _BTree_h_
#define _BTree_h_

#include "Database.h"

class BTree : public Database
{
public:
	//
	// Construction/Destruction
	//
					BTree();
					~BTree();

	int				Open(char *filename, int mode = 0644);
	int				OpenRead(char *filename);
	int				Close();

	int				Put(const DB_Data &key,
					    const DB_Data &data);
	int				Get(const DB_Data &key, DB_Data &data);
	int				Exists(const DB_Data &key);

private:
	void			*data;
	void			*funcs;
};

#endif


