//
// StringList.cc
//
// StringList: Specialized List containing String objects. 
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: StringList.cc,v 1.10 2000/02/19 05:29:03 ghutchis Exp $
//

#include "StringList.h"
#include "htString.h"
#include "List.h"

#include <stdlib.h>


//*****************************************************************************
// StringList::StringList()
//
StringList::StringList()
{
}


//*****************************************************************************
// StringList::~StringList()
//
StringList::~StringList()
{
}

//*****************************************************************************
// void StringList::~SRelease()
//
void StringList::SRelease()
{
    ListCursor  cursor;
    int i;
    int n = Count();

    Start_Get(cursor);
    Object	*obj;
    for(i = 0; i < n && (obj = Get_Next(cursor)); i++) {
	List::Remove(obj);
	delete (String*)obj;
    }
}

//*****************************************************************************
// int StringList::Create(const char *str, char *sep)
//
int StringList::Create(const char *str, const char *sep)
{
    String	word;

    while (str && *str)
    {
	if (strchr(sep, *str))
	{
	  if (word.length())
	  {
	    List::Add(new String(word));
	    word = 0;
	  }
	}
	else
	    word << *str;
	str++;
    }

    //
    // Add the last word to the list
    //
    if (word.length())
	List::Add(new String(word));
    return Count();
}


//*****************************************************************************
// int StringList::Create(const char *str, char sep)
//
int StringList::Create(const char *str, char sep)
{
    String	word;

    while (str && *str)
    {
	if (*str == sep)
	{
	  if (word.length())
	  {
	    List::Add(new String(word));
	    word = 0;
	  }
	}
	else
	    word << *str;
	str++;
    }

    //
    // Add the last word to the list
    //
    if (word.length())
	List::Add(new String(word));
    return Count();
}


//*****************************************************************************
// char *StringList::operator [] (int n)
//
char *StringList::operator [] (int n)
{
    String	*str = (String *) Nth(n);
    if (str)
	return str->get();
    else
	return 0;
}


//*****************************************************************************
// void StringList::Add(char *str)
//
void StringList::Add(char *str)
{
    List::Add(new String(str));
}


//*****************************************************************************
// void StringList::Add(Object *obj)
//
void StringList::Add(Object *obj)
{
    if (!obj)
	Add((char *) 0);
    else
	Add(((String *) obj)->get());
}


//*****************************************************************************
// void StringList::Assign(char *str, int pos)
//
void StringList::Assign(char *str, int pos)
{
    List::Assign(new String(str), pos);
}


//*****************************************************************************
// void StringList::Assign(Object *obj, int pos)
//
void StringList::Assign(Object *obj, int pos)
{
    Assign(((String *) obj)->get(), pos);
}


//*****************************************************************************
// void StringList::Insert(char *str, int pos)
//
void StringList::Insert(char *str, int pos)
{
    List::Insert(new String(str), pos);
}


//*****************************************************************************
// void StringList::Insert(Object *obj, int pos)
//
void StringList::Insert(Object *obj, int pos)
{
    Insert(((String *) obj)->get(), pos);
}


//*******************************************************************************
// int StringList::Remove(int pos)
//
int StringList::Remove(int pos)
{
    Object *o = List::operator[](pos);
    delete o;
    return List::Remove(o);
}


//*****************************************************************************
// int StringList::Remove(Object *obj)
//
int StringList::Remove(Object *obj)
{
    return List::Remove(obj);
}


static int StringCompare(const void *a, const void *b)
{
    String	*sa, *sb;

    sa = *((String **) a);
    sb = *((String **) b);
	
    return strcmp(sa->get(), sb->get());
}


//*****************************************************************************
// void StringList::Sort(int direction)
//
void StringList::Sort(int)
{
    String	**array = new String*[Count()];
    int		i;
    int		n = Count();

    ListCursor  cursor;

    Start_Get(cursor);
    Object	*obj;
    for(i = 0; i < n && (obj = Get_Next(cursor)); i++) {
      array[i] = (String*)obj;
    }

    qsort((char *) array, (size_t) n, (size_t) sizeof(String *),
	  StringCompare);

    Release();

    for (i = 0; i < n; i++)
    {
	List::Add(array[i]);
    }

    delete array;
}

String StringList::Join(char sep) const
{
  String str;
  int i;

  for (i=0; i < number; i++)
  {
      if (str.length())
	str.append(sep);
      str.append(*((const String *) Nth(i)));
  }
  return str;
}
