//
// StringList.cc
//
// Implementation of StringList
//
// $Log: StringList.cc,v $
// Revision 1.5.2.1  1999/11/24 04:21:14  grdetil
// htlib/StringList.cc(Join): Applied Loic's patch to fix memory leak.
//
// Revision 1.5  1999/01/06 16:21:20  bergolth
// fixed bug in StringList::Join
//
// Revision 1.4  1998/12/19 14:39:41  bergolth
// Added StringList::Join and fixed URL::removeIndex.
//
// Revision 1.3  1998/10/02 15:35:04  ghutchis
//
// Fixed bug with multiple delimeters
//
// Revision 1.2  1997/03/24 04:33:22  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: StringList.cc,v 1.5.2.1 1999/11/24 04:21:14 grdetil Exp $";
#endif

#include <stdlib.h>
#include "StringList.h"
#include "htString.h"
#include "List.h"


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
// StringList::StringList(char *str, char sep)
//
StringList::StringList(char *str, char sep)
{
    Create(str, sep);
}


//*****************************************************************************
// StringList::StringList(String &str, char sep)
//
StringList::StringList(String &str, char sep)
{
    Create(str, sep);
}


//*****************************************************************************
// StringList::StringList(char *str, char *sep)
//
StringList::StringList(char *str, char *sep)
{
    Create(str, sep);
}


//*****************************************************************************
// StringList::StringList(String &str, char *sep)
//
StringList::StringList(String &str, char *sep)
{
    Create(str, sep);
}


//*****************************************************************************
// int StringList::Create(String &str, char sep)
//
int StringList::Create(String &str, char sep)
{
    return Create(str.get(), sep);
}


//*****************************************************************************
// int StringList::Create(String &str, char *sep)
//
int StringList::Create(String &str, char *sep)
{
    return Create(str.get(), sep);
}


//*****************************************************************************
// int StringList::Create(char *str, char *sep)
//
int StringList::Create(char *str, char *sep)
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
// int StringList::Create(char *str, char sep)
//
int StringList::Create(char *str, char sep)
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
    String	**array = new String*[number];
    int		i;
    int		n = number;

    listnode	*ln = head;
    for (i = 0; i < n; i++)
    {
	array[i] = (String *) ln->object;
	ln = ln->next;
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

String StringList::Join(char sep)
{
  String str("");
  int i;

  for (i=0; i < number; i++)
  {
      if (str.length())
	str.append(sep);
      str.append(*((String *) Nth(i)));
  }
  return str;
}
