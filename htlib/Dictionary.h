//
// Dictionary.h
//
// Dictionary: This class provides an object lookup table.  
//             Each object in the dictionary is indexed with a string.  
//             The objects can be returned by mentioning their
//             string index.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Dictionary.h,v 1.7 1999/10/08 12:05:20 loic Exp $
//

#ifndef	_Dictionary_h_
#define	_Dictionary_h_

#include "Object.h"
#include "htString.h"
#include "List.h"

class Dictionary;
class DictionaryEntry;

class DictionaryCursor {
 public:
    //
    // Support for the Start_Get and Get_Next routines
    //
    int			currentTableIndex;
    DictionaryEntry	*currentDictionaryEntry;
};

class Dictionary : public Object
{
public:
    //
    // Construction/Destruction
    //
    Dictionary();
    Dictionary(const Dictionary& other);
    Dictionary(int initialCapacity);
    Dictionary(int initialCapacity, float loadFactor);
    ~Dictionary();

    //
    // Adding and deleting items to and from the dictionary
    //
    void		Add(const String& name, Object *obj);
    int			Remove(const String& name);

    //
    // Searching can be done with the Find() member of the array indexing
    // operator
    //
    Object		*Find(const String& name) const;
    Object		*operator[](const String& name) const;
    int			Exists(const String& name) const;

    //
    // We want to be able to go through all the entries in the
    // dictionary in sequence.  To do this, we have the same
    // traversal interface as the List class
    //
    void		Start_Get() { Start_Get(cursor); }
    void		Start_Get(DictionaryCursor& cursor) const;
    //
    // Get the next key
    //
    char		*Get_Next() { return Get_Next(cursor); }
    char		*Get_Next(DictionaryCursor& cursor) const;
    //
    // Get the next entry
    //
    Object              *Get_NextElement() { return Get_NextElement(cursor); }
    Object              *Get_NextElement(DictionaryCursor& cursor) const;
    void		Release();
    void		Destroy();
    int			Count()	const	{ return count; }
    
private:
    DictionaryEntry	**table;
    int			tableLength;
    int			initialCapacity;
    int			count;
    int			threshold;
    float		loadFactor;

    DictionaryCursor	cursor;

    void		rehash();
    void		init(int, float);
    unsigned int	hashCode(const char *key) const;
};

#endif
