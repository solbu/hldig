//
// Dictionary.cc
//
// Implementation of the Dictionary class
//
// $Id: Dictionary.cc,v 1.5.2.2 2001/07/26 03:33:45 grdetil Exp $
//

#include "Dictionary.h"
#include <stdlib.h>

class DictionaryEntry
{
public:
    unsigned int	hash;
    char		*key;
    Object		*value;
    DictionaryEntry	*next;

    ~DictionaryEntry();
    void		release();
};

DictionaryEntry::~DictionaryEntry()
{
    delete [] key;
    delete value;
    if (next)
        delete next;
}

void
DictionaryEntry::release()
{
    value = NULL;		// Prevent the value from being deleted
    if (next)
        next->release();
}


//*********************************************************************
//
Dictionary::Dictionary()
{
    init(101, 10.0f);
}

Dictionary::Dictionary(int initialCapacity, float loadFactor)
{
    init(initialCapacity, loadFactor);
}

Dictionary::Dictionary(int initialCapacity)
{
    init(initialCapacity, 0.75f);
}


//*********************************************************************
//
Dictionary::~Dictionary()
{
    for (int i = 0; i < tableLength; i++)
    {
	delete table[i];
    }
    delete [] table;
}


//*********************************************************************
//
void
Dictionary::Destroy()
{
    for (int i = 0; i < tableLength; i++)
    {
	if (table[i] != NULL)
	{
	    delete table[i];
	    table[i] = NULL;
	}
    }
    count = 0;
}

//*********************************************************************
//
void
Dictionary::Release()
{
    for (int i = 0; i < tableLength; i++)
    {
	if (table[i] != NULL)
	{
	    table[i]->release();
	    delete table[i];
	    table[i] = NULL;
	}
    }
    count = 0;
}


//*********************************************************************
//
void
Dictionary::init(int initialCapacity, float loadFactor)
{
    if (initialCapacity <= 0)
	initialCapacity = 101;
    if (loadFactor <= 0.0)
	loadFactor = 0.75f;
    Dictionary::loadFactor = loadFactor;
    table = new DictionaryEntry*[initialCapacity];
    for (int i = 0; i < initialCapacity; i++)
    {
	table[i] = NULL;
    }
    threshold = (int)(initialCapacity * loadFactor);
    tableLength = initialCapacity;
    count = 0;
}

//*********************************************************************
//
unsigned int
Dictionary::hashCode(char *key)
{
    char *test;
    long  conv_key = strtol(key,  &test, 10);
    if (key && *key && !*test) // Conversion succeeded
      return conv_key;
    unsigned int	h = 0;
    int			length = strlen(key);

    if (length < 16)
    {
	for (int i = length; i > 0; i--)
	{
	    h = (h * 37) + *key++;
	}
    }
    else
    {
	int	skip = length / 8;
	for (int i = length; i > 0; i -= skip, key += skip)
	{
	    h = (h * 39) + *key;
	}
    }
    return h;
}


//*********************************************************************
//   Add an entry to the hash table.  This will *not* delete the
//   data associated with an already existing key.  Use the Replace
//   method for that function.
//
void
Dictionary::Add(char *name, Object *obj)
{
    unsigned int	hash = hashCode(name);
    int			index = hash % tableLength;
    DictionaryEntry	*e;
    
    for (e = table[index]; e != NULL; e = e->next)
    {
	if (e->hash == hash && strcmp(e->key, name) == 0)
	{
	    delete e->value;
	    e->value = obj;
	    return;
	}
    }

    if (count >= threshold)
    {
	rehash();
	Add(name, obj);
	return;
    }

    e = new DictionaryEntry();
    e->hash = hash;
    e->key = strdup(name);
    e->value = obj;
    e->next = table[index];
    table[index] = e;
    count++;
}


//*********************************************************************
//   Remove an entry from the hash table.
//
int
Dictionary::Remove(char *name)
{
    if (!count)
      return 0;

    unsigned int	hash = hashCode(name);
    int			index = hash % tableLength;
    DictionaryEntry	*e, *prev;

    for (e = table[index], prev = NULL; e != NULL; prev = e, e = e->next)
    {
	if (hash == e->hash && strcmp(e->key, name) == 0)
	{
	    if (prev != NULL)
	    {
		prev->next = e->next;
	    }
	    else
	    {
		table[index] = e->next;
	    }
	    count--;
	    e->next = NULL; // FIXED: Otherwise ALL entries after e will be deleted by DictionaryEntry's dtor
            delete e;
	    return 1;
	}
    }
    return 0;
}


//*********************************************************************
//
Object *Dictionary::Find(char *name)
{
    if (!count)
	return NULL;

    unsigned int	hash = hashCode(name);
    int			index = hash % tableLength;
    DictionaryEntry	*e;

    for (e = table[index]; e != NULL; e = e->next)
    {
	if (e->hash == hash && strcmp(e->key, name) == 0)
	{
	    return e->value;
	}
    }
    return NULL;
}


//*********************************************************************
//
Object *Dictionary::operator[](char *name)
{
    return Find(name);
}


//*********************************************************************
//
int Dictionary::Exists(char *name)
{
    if (!count)
      return 0;

    unsigned int	hash = hashCode(name);
    int			index = hash % tableLength;
    DictionaryEntry	*e;

    for (e = table[index]; e != NULL; e = e->next)
    {
	if (e->hash == hash && strcmp(e->key, name) == 0)
	{
	    return 1;
	}
    }
    return 0;
}


//*********************************************************************
//
void
Dictionary::rehash()
{
    DictionaryEntry	**oldTable = table;
    int			oldCapacity = tableLength;

    int			newCapacity;
    DictionaryEntry	*e;
    int			i, index;
    
    newCapacity = count > oldCapacity ? count * 2 + 1 : oldCapacity * 2 + 1;

    DictionaryEntry	**newTable = new DictionaryEntry*[newCapacity];

    for (i = 0; i < newCapacity; i++)
    {
	newTable[i] = NULL;
    }

    threshold = (int) (newCapacity * loadFactor);
    table = newTable;
    tableLength = newCapacity;
    
    for (i = oldCapacity; i-- > 0;)
    {
	for (DictionaryEntry *old = oldTable[i]; old != NULL;)
	{
	    e = old;
	    old = old->next;
	    index = e->hash % newCapacity;
	    e->next = newTable[index];
	    newTable[index] = e;
	}
    }
    delete [] oldTable;
}


//*********************************************************************
//
void
Dictionary::Start_Get()
{
    currentTableIndex = -1;
    currentDictionaryEntry = NULL;
}


//*********************************************************************
//
char *
Dictionary::Get_Next()
{
    while (currentDictionaryEntry == NULL ||
	   currentDictionaryEntry->next == NULL)
    {
	currentTableIndex++;

	if (currentTableIndex >= tableLength)
	{
	    currentTableIndex--;
	    return NULL;
	}

	currentDictionaryEntry = table[currentTableIndex];

	if (currentDictionaryEntry != NULL)
	{
	    return currentDictionaryEntry->key;
	}
    }

    currentDictionaryEntry = currentDictionaryEntry->next;
    return currentDictionaryEntry->key;
}

//*********************************************************************
//
Object *
Dictionary::Get_NextElement()
{
    while (currentDictionaryEntry == NULL ||
	   currentDictionaryEntry->next == NULL)
    {
	currentTableIndex++;

	if (currentTableIndex >= tableLength)
	{
	    currentTableIndex--;
	    return NULL;
	}

	currentDictionaryEntry = table[currentTableIndex];

	if (currentDictionaryEntry != NULL)
	{
	    return currentDictionaryEntry->value;
	}
    }

    currentDictionaryEntry = currentDictionaryEntry->next;
    return currentDictionaryEntry->value;
}
