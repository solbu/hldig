//
// BTree.cc
//
// Implementation of BTree
//
// $Log: BTree.cc,v $
// Revision 1.3  1999/01/23 01:25:02  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.2  1998/06/22 04:33:18  turtle
// New Berkeley database stuff
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: BTree.cc,v 1.3 1999/01/23 01:25:02 hp Exp $";
#endif

#include "BTree.h"
#include <sys/file.h>
#include <sys/types.h>
#include <limits.h>
#include <db.h>


//*******************************************************************************
// BTree::BTree()
//
BTree::BTree()
{
    data = 0;
}


//*******************************************************************************
// BTree::~BTree()
//
BTree::~BTree()
{
    Close();
    delete data;
}


//*******************************************************************************
// int BTree::Open(char *filename, int mode)
//
int BTree::Open(char *filename, int mode)
{
    BTREEINFO	*bi = new BTREEINFO;
    Close();

    data = (void *) bi;
    bi->flags = 0;
    bi->cachesize = 2 * 1024 * 1024;
    bi->maxkeypage = 0;
    bi->minkeypage = 0;
    bi->psize = 0;
    bi->compare = 0;
    bi->prefix = 0;
    bi->lorder = 0;

    funcs = (DB *) dbopen(filename, O_RDWR | O_CREAT, mode, DB_BTREE, data);
    if (funcs)
	return 0;
    else
	return -1;
}


//*******************************************************************************
// int BTree::OpenRead(char *filename)
//
int BTree::OpenRead(char *filename)
{
    BTREEINFO	*bi = new BTREEINFO;
    Close();

    data = (void *) bi;
    bi->flags = 0;
    bi->cachesize = 2 * 1024 * 1024;
    bi->maxkeypage = 0;
    bi->minkeypage = 0;
    bi->psize = 0;
    bi->compare = 0;
    bi->prefix = 0;
    bi->lorder = 0;

    funcs = (DB *) dbopen(filename, O_RDONLY, 0, DB_BTREE, data);
    if (funcs)
	return 0;
    else
	return -1;
}


//*******************************************************************************
// int BTree::Close()
//
int BTree::Close()
{
    if (funcs)
    {
	int status = ((DB *) funcs)->close((DB *) funcs);
	delete data;
	data = 0;
	return status;
    }
    return 0;
}


//*******************************************************************************
// int BTree::Put(const DB_Data &key, const DB_Data &data)
//
int BTree::Put(const DB_Data &key, const DB_Data &data)
{
    if (!funcs)
    {
	return -1;
    }
    return ((DB *) funcs)->put((DB *) funcs, (DBT *) &key, (DBT *) &data, 0);
}


//*******************************************************************************
// int BTree::Get(const DB_Data &key, DB_Data &data)
//
int BTree::Get(const DB_Data &key, DB_Data &data)
{
    if (!funcs)
    {
	return -1;
    }
    return ((DB *) funcs)->get((DB *) funcs, (DBT *) &key, (DBT *) &data, 0);
}


//*******************************************************************************
// int BTree::Exists(const DB_Data &key)
//
int BTree::Exists(const DB_Data &key)
{
    if (!funcs)
    {
	return 0;
    }

    DB_Data	data;
    int		status;
    
    status = ((DB *) funcs)->get((DB *) funcs, (DBT *) &key, (DBT *) &data, 0);
    return status < 0 ? 0 : 1;
}


