//
// DocumentDB.h
//
// This class is the interface to the database of document references.
// This database is only used while digging.  An extract of this database is used
// for searching.  This is because digging requires a different index than searching.
//
// $Id: DocumentDB.h,v 1.1.1.1 1997/02/03 17:11:07 turtle Exp $
//
// $Log: DocumentDB.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#ifndef _DocumentDB_h_
#define _DocumentDB_h_

#include "DocumentRef.h"
#include <List.h>
#include <Database.h>


class DocumentDB
{
public:
	//
	// Construction/Destruction
	//
					DocumentDB();
					~DocumentDB();

	//
	// The database used for searching is generated from our internal database:
	//
	int				CreateSearchDB(char *filename);

	//
	// Standard database operations
	//
	int				Open(char *filename);
	int				Read(char *filename);
	int				Close();

	int				Add(DocumentRef &);
	DocumentRef		*operator [] (char *url);
	int				Exists(char *url);
	int				Delete(char *url);

	//
	// The database keeps track of document ids.  Here is a way to get the next
	// document id.
	//
	int				NextDocID()					{return nextDocID++;}

	//
	// We will need to be able to iterate over the complete database.
	//
	List			*URLs();		// This returns a list of all the URLs

private:
	Database		*dbf;
	int				isopen;
	int				isread;
	int				nextDocID;
};

#endif


