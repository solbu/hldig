//
// htString.h
//
// htString: (implementation in String.cc) Just Another String class.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htString.h,v 1.24.2.1 2005/10/12 18:12:27 aarnone Exp $
//
#ifndef __String_h
#define __String_h

#include "Object.h"

#include <stdarg.h>
#include <stdio.h>
#include <string>

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

#ifdef UNICODE
  #define char_t wchar_t
#else
  #define char_t char
#endif

class htString : public Object
{
public:
    htString()	{ Length = 0; Allocated = 0; Data = 0; } // Create an empty string
    htString(int init);                 // initial allocated length
    htString(const char *s);            // from null terminated s
    htString(const char *s, int len);   // from s with length len
    htString(const htString &s);          // Copy constructor

    //
    // This can be used for performance reasons if it is known the
    // String will need to grow.
    //
    htString(const htString &s, int allocation_hint);
	
    ~htString();

    inline int      length() const;
    char            *get();
    const char		*get() const;
    operator        char*() { return get(); }
    operator        const char*() const { return get(); }

    //
    // Interpretation
    //
    int             as_integer(int def = 0) const;
    double          as_double(double def = 0) const;
    int             empty() const { return length() == 0; }

    //
    // If it is not posible to use the constructor with an initial
    // allocation size, use the following member to set the size.
    //
    void            allocate(int init)	{reallocate_space(init);}

    //	
    // allocate space for a new char *, and copy the String in.
    //
    char            *new_char() const;

    //
    // Assignment
    //
    inline htString&  set(const char *s, int l) { trunc(); append(s, l); return *this; }
    inline htString&  set(char *s) { trunc(); append(s, strlen(s)); return *this; }
    void            operator = (const htString &s);
    void            operator = (const char *s);
    inline void     operator += (const htString &s) { append(s); }
    inline void     operator += (const char *s) { append(s); }

    //
    // Appending
    //
    inline htString   &operator << (const char *);
    inline htString   &operator << (char);
    inline htString   &operator << (unsigned char c) {return *this<<(char)c;}
    htString          &operator << (const htString &);
    htString          &operator << (const htString *s) {return *this << *s;}
    inline htString   &operator << (short i) {return *this<<(int)i;}
    htString          &operator << (int);
    htString          &operator << (unsigned int);
    htString          &operator << (long);

    //
    // Access to specific characters
    //
    inline char		&operator [] (int n);
    inline char		operator [] (int n) const;
    inline char		Nth (int n) { return (*this)[n]; }
    inline char		last() const { return Length > 0 ? Data[Length - 1] : '\0'; }

    //
    // Removing
    //
    char            operator >> (char c);
									
    //
    // Comparison
    //  Return:
    //	 0 : 'this' is equal to 's'.
    //	-1 : 'this' is less than 's'.
    //	 1 : 'this' is greater than 's'.
    //
    int             compare(const Object& s) const { return compare((const htString&)s); }
    int             compare(const htString& s) const;
    int             nocase_compare(const htString &s) const;

    //
    // Searching for parts
    //
    int             lastIndexOf(char c) const;
    int             lastIndexOf(char c, int pos) const;
    int             indexOf(char c) const;
    int             indexOf(char c, int pos) const;
    int             indexOf(const char *) const;
    int             indexOf(const char *, int pos) const;
    
    //
    // Manipulation
    //
    void            append(const htString &s);
    void            append(const char *s);
    void            append(const char *s, int n);
    void            append(char ch);

    inline htString   &trunc() { Length = 0; return *this; }
    htString          &chop(int n = 1);
    htString          &chop(char ch = '\n');
    htString          &chop(const char *str = (char *)"\r\n");

    //
    // SubStrings
    //
    // The string starting at postion 'start' and length 'len'.
    //
    htString          sub(int start, int len) const;
    htString          sub(int start) const;

    //
    // IO
    //
    int             Write(int fd) const;

#ifndef NOSTREAM
    void            debug(ostream &o);
#endif /* NOSTREAM */

    //
    // Non-member operators
    //
    friend htString operator +  (const htString &a, const htString &b);
    friend int      operator == (const htString &a, const htString &b);
    friend int      operator != (const htString &a, const htString &b);
    friend int      operator <  (const htString &a, const htString &b);
    friend int      operator >  (const htString &a, const htString &b);
    friend int      operator <= (const htString &a, const htString &b);
    friend int      operator >= (const htString &a, const htString &b);

#ifndef NOSTREAM
    friend ostream  &operator << (ostream &o, const htString &s);

    friend istream  &operator >> (istream &in, htString &line);
#endif /* NOSTREAM */

    int             readLine(FILE *in);

    int             lowercase();
    int             uppercase();

    void            replace(char c1, char c2);
    int             remove(const char *);

    Object          *Copy() const { return new htString(*this); }

    //
    // Persistent storage support
    //
    void            Serialize(String &);
    void            Deserialize(String &, int &);

private:
    basic_string<char_t> Data;      // The actual string

    void            copy_data_from(const char *s, int len, int dest_offset = 0);
    void            copy(const char *s, int len, int allocation_hint);

    //
    // Possibly make Data bigger.
    //
    void            reallocate_space(int len);

    //
    // Allocate some space for the data.  Delete Data if it
    // has been allocated.
    //
    void            allocate_space(int len);
    // Allocate some space without rounding
    void            allocate_fix_space(int len);

    friend          class StringIndex;
};

extern char *form(const char *, ...);
extern char *vform(const char *, va_list);

//
// Inline methods.
//
inline htString &String::operator << (const char *str)
{
    append(str);
    return *this;
}

inline htString &String::operator << (char ch)
{
    append(ch);
    return *this;
}

inline int htString::length() const
{
    return Length;
}

inline char	String::operator [] (int n) const
{
  if(n < 0) n = Length + n;
  if(n >= Length || n < 0) return '\0';

  return Data[n];
}

static char null = '\0';

inline char	&String::operator [] (int n)
{
  if(n < 0) n = Length + n;
  if(n >= Length || n < 0) return null;

  return Data[n];
}

//
// Non friend, non member operators
//
#endif
