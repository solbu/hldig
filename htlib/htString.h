//
// htString.h
//
// htString: (implementation in String.cc) Just Another String class.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htString.h,v 1.18.2.9 2000/10/10 03:15:42 ghutchis Exp $
//
#ifndef __String_h
#define __String_h

#include "Object.h"

#include <stdarg.h>
#include <stdio.h>
#ifndef NOSTREAM
#include <iostream.h>
#endif /* NOSTREAM */

class String : public Object
{
public:
    String()	{ Length = 0; Allocated = 0; Data = 0; } // Create an empty string
    String(int init);			// initial allocated length
    String(const char *s);		// from null terminated s
    String(const char *s, int len);	// from s with length len
    String(const String &s);            // Copy constructor

    //
    // This can be used for performance reasons if it is known the
    // String will need to grow.
    //
    String(const String &s, int allocation_hint);
	
    ~String();

    inline int		length() const;
    char		*get();
    const char		*get() const;
    operator 		char*()	{ return get(); }
    operator 		const char*() const { return get(); }
    operator 		int() const;

    //
    // Interpretation
    //
    int			as_integer(int def = 0) const;
    double		as_double(double def = 0) const;
    int			empty() const { return length() == 0; }

    //
    // If it is not posible to use the constructor with an initial
    // allocation size, use the following member to set the size.
    //
    void		allocate(int init)	{reallocate_space(init);}

    //	
    // allocate space for a new char *, and copy the String in.
    //
    char		*new_char() const;

    //
    // Assignment
    //
    inline String&	set(const char *s, int l) { trunc(); append(s, l); return *this; }
    inline String&	set(char *s) { trunc(); append(s, strlen(s)); return *this; }
    void		operator = (const String &s);
    void		operator = (const char *s);
    inline void		operator += (const String &s) { append(s); }
    inline void		operator += (const char *s) { append(s); }

    //
    // Appending
    //
    inline String	&operator << (const char *);
    inline String	&operator << (char);
    inline String	&operator << (unsigned char c) {return *this<<(char)c;}
    String		&operator << (int);
    String		&operator << (unsigned int);
    String		&operator << (long);
    inline String	&operator << (short i)		{return *this<<(int)i;}
    String		&operator << (const String &);
    String		&operator << (const String *s)	{return *this << *s;}

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
    char		operator >> (char c);
									
    //
    // Comparison
    //  Return:
    //	 0 : 'this' is equal to 's'.
    //	-1 : 'this' is less than 's'.
    //	 1 : 'this' is greater than 's'.
    //
    int			compare(const Object& s) const { return compare((const String&)s); }
    int			compare(const String& s) const;
    int			nocase_compare(const String &s) const;

    //
    // Searching for parts
    //
    int			lastIndexOf(char c) const;
    int			lastIndexOf(char c, int pos) const;
    int			indexOf(char c) const;
    int			indexOf(char c, int pos) const;
    int			indexOf(const char *) const;
    int			indexOf(const char *, int pos) const;
    
    //
    // Manipulation
    //
    void		append(const String &s);
    void		append(const char *s);
    void		append(const char *s, int n);
    void		append(char ch);

    inline String	&trunc() { Length = 0; return *this; }
    String		&chop(int n = 1);
    String		&chop(char ch = '\n');
    String		&chop(const char *str = "\r\n");

    //
    // SubStrings
    //
    // The string starting at postion 'start' and length 'len'.
    //
    String		sub(int start, int len) const;
    String		sub(int start) const;

    //
    // IO
    //
    int			Write(int fd) const;

#ifndef NOSTREAM
    void		debug(ostream &o);
#endif /* NOSTREAM */

    //
    // Non-member operators
    //
    friend String	operator +  (const String &a, const String &b);
    friend int		operator == (const String &a, const String &b);
    friend int		operator != (const String &a, const String &b);
    friend int		operator <  (const String &a, const String &b);
    friend int		operator >  (const String &a, const String &b);
    friend int		operator <= (const String &a, const String &b);
    friend int		operator >= (const String &a, const String &b);

#ifndef NOSTREAM
    friend ostream	&operator << (ostream &o, const String &s);

    friend istream	&operator >> (istream &in, String &line);
#endif /* NOSTREAM */

    int			readLine(FILE *in);

    int			lowercase();
    int			uppercase();

    void		replace(char c1, char c2);
    int			remove(const char *);

    Object		*Copy() const { return new String(*this); }

    //
    // Persistent storage support
    //
    void		Serialize(String &);
    void		Deserialize(String &, int &);

private:
    int			Length;		// Current Length
    int			Allocated;	// Total space allocated
    char		*Data;		// The actual contents

    void		copy_data_from(const char *s, int len, int dest_offset = 0);
    void		copy(const char *s, int len, int allocation_hint);

    //
    // Possibly make Data bigger.
    //
    void		reallocate_space(int len);

    //
    // Allocate some space for the data.  Delete Data if it
    // has been allocated.
    //
    void		allocate_space(int len);
    // Allocate some space without rounding
    void 	allocate_fix_space(int len);

    friend		class StringIndex;
};

extern char *form(const char *, ...);
extern char *vform(const char *, va_list);

//
// Inline methods.
//
inline String &String::operator << (const char *str)
{
    append(str);
    return *this;
}

inline String &String::operator << (char ch)
{
    append(ch);
    return *this;
}

inline int String::length() const
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
