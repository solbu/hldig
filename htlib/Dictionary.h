//
// Dictionary.h
//
// This class provides an object lookup table.  Each object in the dictionary
// is indexed with a string.  The objects can be returned by mentioning their
// string index.
//
// $Id: Dictionary.h,v 1.2.2.1 1999/03/23 00:46:45 grdetil Exp $
//
//
//
#ifndef	_Dictionary_h_
#define	_Dictionary_h_

#include "Object.h"
#include "List.h"

class DictionaryEntry;

class Dictionary : public Object
{
public:
    //
    // Construction/Destruction
    //
    Dictionary();
    Dictionary(int initialCapacity);
    Dictionary(int initialCapacity, float loadFactor);
    ~Dictionary();

    //
    // Adding and deleting items to and from the dictionary
    //
    void		Add(char *name, Object *obj);
    int			Remove(char *name);

    //
    // Searching can be done with the Find() member of the array indexing
    // operator
    //
    Object		*Find(char *name);
    Object		*operator[](char *name);
    int			Exists(char *name);

    //
    // We want to be able to go through all the entries in the
    // dictionary in sequence.  To do this, we have the same
    // traversal interface as the List class
    //
    char		*Get_Next();
    void		Start_Get();
    Object              *Get_NextElement();
    void		Release();
    void		Destroy();
    int			Count()		{return count;}
    
private:
    DictionaryEntry	**table;
    int			tableLength;
    int			initialCapacity;
    int			count;
    int			threshold;
    float		loadFactor;

    //
    // Support for the Start_Get and Get_Next routines
    //
    int			currentTableIndex;
    DictionaryEntry	*currentDictionaryEntry;
    
    void		rehash();
    void		init(int, float);
    unsigned int	hashCode(char *key);
};

#endif
