//
// $Id: String.h,v 1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: String.h,v $
// Revision 1.1  1997/02/03 17:11:04  turtle
// Initial revision
//
//
#ifndef __String_h
#define __String_h

#include "Object.h"
#include <stdarg.h>

class ostream;

class String : public Object
{
public:
    String();				// Create an empty string
    String(int init);			// initial allocated length
    String(char *s);			// from null terminated s
    String(char *s, int len);		// from s with length len
    String(String *s);			// Copy constructor

    //
    // This can be used for performance reasons if it is known the
    // String will need to grow.
    //
    String(String &s, int allocation_hint = 0);
	
    ~String();

    int			length() const;
    char		*get() const;
    operator 		char*()	{return get();}

    //
    // Interpretation
    //
    int			as_integer(int def = 0);

    //
    // If it is not posible to use the constructor with an initial
    // allocation size, use the following member to set the size.
    //
    void		allocate(int init)	{reallocate_space(init);}

    //	
    // allocate space for a new char *, and cope the String in.
    //
    char		*new_char() const;

    //
    // Assignment
    //
    void		operator = (String &s);
    void		operator = (char *s);
    void		operator += (String &s);
    void		operator += (char *s);

    //
    // Appending
    //
    String		&operator << (char *);
    String		&operator << (char);
    String		&operator << (unsigned char c) {return *this<<(char)c;}
    String		&operator << (int);
    String		&operator << (long);
    String		&operator << (short i)		{return *this<<(int)i;}
    String		&operator << (String &);
    String		&operator << (String *s)	{return *this << *s;}

    //
    // Access to specific characters
    //
    char		&operator [] (int n);
    char		last();

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
    int			compare(Object *obj);
    int			nocase_compare(String &s);

    //
    // Searching for parts
    //
    int			lastIndexOf(char c);
    int			lastIndexOf(char c, int pos);
    int			indexOf(char c);
    int			indexOf(char c, int pos);
    int			indexOf(char *);
    int			indexOf(char *, int pos);
    
    //
    // Manipulation
    //
    void		append(String &s);
    void		append(char *s);
    void		append(char *s, int n);
    void		append(char ch);

    String		&chop(int n = 1);
    String		&chop(char ch = '\n');
    String		&chop(char *str = "\r\n");

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
    int			write(int fd) const;

    void		debug(ostream &o);

    //
    // Non-member operators
    //
    friend String	operator +  (String &a, String &b);
    friend int		operator == (String &a, String &b);
    friend int		operator != (String &a, String &b);
    friend int		operator <  (String &a, String &b);
    friend int		operator >  (String &a, String &b);
    friend int		operator <= (String &a, String &b);
    friend int		operator >= (String &a, String &b);

    friend ostream	&operator << (ostream &o, String &s);

    void		lowercase();
    void		uppercase();

    void		replace(char c1, char c2);
    void		remove(char *);

    Object		*Copy();

    //
    // Persistent storage support
    //
    void		Serialize(String &);
    void		Deserialize(String &, int &);

private:
    int			Length;		// Current Length
    int			Allocated;	// Total space allocated
    char		*Data;		// The actual contents

    void		copy_data_from(char *s, int len, int dest_offset = 0);
    void		copy(char *s, int len, int allocation_hint);

    //
    // Possibly make Data bigger.
    //
    void		reallocate_space(int len);

    //
    // Allocate some space for the data.  Delete Data if it
    // has been allocated.
    //
    void		allocate_space(int len);
	
    friend		class StringIndex;
};

extern char *form(char *, ...);
extern char *vform(char *, va_list);

//
// Inline methods.
//
inline int String::length() const
{
    return Length;
}

inline void String::operator += (String &s)
{
    append(s);
}

inline void String::operator += (char *s)
{
    append(s);
}

//
// Non friend, non member operators
//
#endif
