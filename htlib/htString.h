//
// Just Another String class.
//
// $Id: htString.h,v 1.8 1999/08/02 09:04:10 angus Exp $
//
#ifndef __String_h
#define __String_h

#include "Object.h"
#include <stdarg.h>

class ostream;

class String : public Object
{
public:
    String()	{Length = 0,Allocated = 0;} // Create an empty string
    String(int init);			// initial allocated length
    String(char *s);			// from null terminated s
    String(char *s, int len);		// from s with length len
    String(const String &s);            // Copy constructor

    //
    // This can be used for performance reasons if it is known the
    // String will need to grow.
    //
    String(const String &s, int allocation_hint);
	
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
    // allocate space for a new char *, and copy the String in.
    //
    char		*new_char() const;

    //
    // Assignment
    //
    void		operator = (const String &s);
    void		operator = (char *s);
    void		operator += (String &s);
    void		operator += (char *s);

    //
    // Appending
    //
    inline String		&operator << (char *);
    inline String		&operator << (char);
    String		&operator << (unsigned char c) {return *this<<(char)c;}
    String		&operator << (int);
    String		&operator << (unsigned int);
    String		&operator << (long);
    String		&operator << (short i)		{return *this<<(int)i;}
    String		&operator << (String &);
    String		&operator << (String *s)	{return *this << *s;}

    //
    // Access to specific characters
    //
    char		&operator [] (int n);
    char		Nth (int n);
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
    // Allocate some space without rounding
    void 	allocate_fix_space(int len);

    friend		class StringIndex;
};

extern char *form(char *, ...);
extern char *vform(char *, va_list);

//
// Inline methods.
//
inline String &String::operator << (char *str)
{
    append(str);
    return *this;
}

inline String &String::operator << (char ch)
{
    append(ch);
    return *this;
}

#ifdef TOTO
inline char *String::get() const
{
    if (Data == 0)
	    return 0;
    Data[Length] = '\0';	// We always leave room for this.
    return Data;
}
#endif

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
