//
// BTree.h
//
// $Id: BTree.h,v 1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: BTree.h,v $
// Revision 1.1  1997/02/03 17:11:04  turtle
// Initial revision
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

	int				Put(DB_Data &key, DB_Data &data);
	int				Get(DB_Data &key, DB_Data &data);
	int				Exists(DB_Data &key);

private:
	void			*data;
	void			*funcs;
};

#endif


