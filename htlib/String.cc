//
// Change history.
//
// Who	When		What
// ---	----		----
// AWS	10/13/93	Fixed the constructors and operator = routines so that a NULL can be passed
//
// $Log: String.cc,v $
// Revision 1.7  1998/08/03 16:50:41  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.6  1998/07/16 15:15:26  ghutchis
//
// Added patch from Stephan Muehlstrasser <smuehlst@Rational.Com> to fix
// delete syntax and a memory leak.
//
// Revision 1.5  1998/06/22 04:33:24  turtle
// New Berkeley database stuff
//
// Revision 1.4  1998/05/26 03:58:09  turtle
// Got rid of compiler warnings.
//
// Revision 1.3  1997/03/24 04:33:21  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.2  1997/02/24 17:52:52  turtle
// Applied patches supplied by "Jan P. Sorensen" <japs@garm.adm.ku.dk> to make
// ht://Dig run on 8-bit text without the global unsigned-char option to gcc.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: String.cc,v 1.7 1998/08/03 16:50:41 ghutchis Exp $";
#endif


#include "htString.h"

#include <unistd.h>
#include <stream.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

const int MinimumAllocationSize = 64;	// Should be power of two.
int next_power_of_2(int n);

String::String()
{
    Length = 0;
    Allocated = 0;
    Data = 0;
}

String::String(int init)
{
    Length = 0;
    Allocated = init;
    Data = new char[init];
}

String::String(char *s)
{
    Data = 0;

    int	len = 0;
    if (s)
	len = strlen(s);
    copy(s, len, len);
}

String::String(char *s, int len)
{
    Data = 0;
    copy(s, len, len);
}

String::String(String *s)
{
    Data = 0;
    Length = 0;

    if (s)
	copy(s->Data, s->length(), s->length());
}

//
// This can be used for performance reasons if it is known the
// String will need to grow.
//
String::String(const String &s, int allocation_hint)
{
    Data = 0;

    if (allocation_hint < s.length())
	allocation_hint = s.length();

    copy(s.Data, s.length(), allocation_hint);
}

String::~String()
{
    if (Data)
	delete [] Data;
}

void String::operator = (String &s)
{
    Length = s.length();
    allocate_space(Length);
    copy_data_from(s.Data, Length);
}

void String::operator = (char *s)
{
    if (s)
	Length = strlen(s);
    else
	Length = 0;
    allocate_space(Length);
    copy_data_from(s, Length);	
}

void String::append(String &s)
{
    if (s.length() == 0)
	return;
    int	new_len = Length + s.length();

    reallocate_space(new_len);
    copy_data_from(s.Data, s.length(), Length);
    Length = new_len;
}

void String::append(char *s)
{
    if (!s)
	return;
    int	slen = strlen(s);
    int	new_len = Length + slen;
	
    reallocate_space(new_len);
    copy_data_from(s, slen, Length);
    Length = new_len;
}

void String::append(char *s, int slen)
{
    if (!s)
	return;
    int	new_len = Length + slen;
	
    reallocate_space(new_len);
    copy_data_from(s, slen, Length);
    Length = new_len;
}

void String::append(char ch)
{
    append(&ch, 1);
}

int String::compare(Object *obj)
{
    String	*s = (String *) obj;
    int	len;
    int	result;
    char	*p1 = Data;
    char	*p2 = s->Data;

    len = Length;
    result = 0;

    if (Length > s->Length)
    {
	result = 1;
	len = s->Length;
    }
    else if (Length < s->Length)
	result = -1;

    while (len)
    {
	if (*p1 > *p2)
	    return 1;
	if (*p1 < *p2)
	    return -1;
	p1++;
	p2++;
	len--;
    }
    //
    // Strings are equal up to the shortest length.
    // The result depends upon the length difference.
    //
    return result;
}

int String::write(int fd) const
{
    int	left = Length;
    char	*wptr = Data;
	
    while (left)
    {
	int result = ::write(fd, wptr, left);
		
	if (result < 0)
	    return result;
		
	left -= result;
    }
    return left;
}

char *String::get() const
{
    if (Data == 0)
	return 0;
    Data[Length] = '\0';	// We always leave room for this.
    return Data;
}

char *String::new_char() const
{
    char	*r;
    if (Data == 0)
    {
	r = new char[1];
	*r = '\0';
	return r;
    }
    Data[Length] = '\0';	// We always leave room for this.
    r = new char[Length + 1];
    strcpy(r, Data);
    return r;
}


int String::as_integer(int def)
{
    if (Length <= 0)
	return def;
    Data[Length] = '\0';
    return atoi(Data);
}


String String::sub(int start, int len) const
{
    if (start > Length)
	return 0;

    if (len > Length - start)
	len = Length - start;

    return String(Data + start, len);
}

String String::sub(int start) const
{
    return sub(start, Length - start);
}

int String::indexOf(char *str)
{
    int		len = strlen(str);
    int		i;
    
    //
    // Set the first char after string end to zero to prevent finding
    // substrings including symbols after actual end of string
    //
    Data[Length] = '\0';
    
    for (i = 0; i < Length; i++)
    {
	if (strncmp(&Data[i], str, len) == 0)
	    return i;
    }
    return -1;
}

int String::indexOf(char ch)
{
    int		i;
    for (i = 0; i < Length; i++)
    {
	if (Data[i] == ch)
	    return i;
    }
    return -1;
}

int String::lastIndexOf(char ch, int pos)
{
    if (pos >= Length)
	return -1;
    while (pos >= 0)
    {
	if (Data[pos] == ch)
	    return pos;
	pos--;
    }
    return -1;
}

String &String::operator << (char *str)
{
    append(str);
    return *this;
}

String &String::operator << (char ch)
{
    append(&ch, 1);
    return *this;
}

String &String::operator << (int i)
{
    char	str[20];
    sprintf(str, "%d", i);
    append(str);
    return *this;
}

String &String::operator << (long l)
{
    char	str[20];
    sprintf(str, "%ld", l);
    append(str);
    return *this;
}

String &String::operator << (String &s)
{
    append(s.get(), s.length());
    return *this;
}

char	String::operator >> (char c)
{
    c = '\0';
	
    if (Data && *Data)
    {
	c = Data[Length - 1];
	Data[Length - 1] = '\0';
	Length--;
    }

    return c;
}


char String::last()
{
    if (Length > 0)
	return Data[Length - 1];
    else
	return 0;
}


char	&String::operator [] (int n)
{
    static char	null = 0;
    if (n >= Length)
	return null;
    else if (n < 0)
	return (*this)[Length + n];
    else
	return Data[n];
}


void String::lowercase()
{
    for (int i = 0; i < Length; i++)
    {
//		if (isupper(Data[i]))
	Data[i] = tolower((unsigned char)Data[i]);
    }
}


void String::uppercase()
{
    for (int i = 0; i < Length; i++)
    {
	if (islower(Data[i]))
	    Data[i] = toupper((unsigned char)Data[i]);
    }
}


void String::replace(char c1, char c2)
{
    for (int i = 0; i < Length; i++)
	if (Data[i] == c1)
	    Data[i] = c2;
}


void String::remove(char *chars)
{
    if (Length <= 0)
	return;

    char	*good, *bad;
    int		skipped = 0;

    good = bad = Data;
    for (int i = 0; i < Length; i++)
    {
	if (strchr(chars, *bad))
	    skipped++;
	else
	    *good++ = *bad;
	bad++;
    }
    Length -= skipped;
}


Object *String::Copy()
{
    return new String(this);
}


String &String::chop(int n)
{
    Length -= n;
    if (Length < 0)
	Length = 0;
    return *this;
}


String &String::chop(char ch)
{
    if (Data)
    {
	while (Length > 0 && Data[Length - 1] == ch)
	    Length--;
    }
    return *this;
}


String &String::chop(char *str)
{
    if (Data)
    {
	while (Length > 0 && strchr(str, Data[Length - 1]))
	    Length--;
    }
    return *this;
}


void String::Serialize(String &dest)
{
    dest.append((char *) &Length, sizeof(Length));
    dest.append(get(), Length);
}


void String::Deserialize(String &source, int &index)
{
    memcpy((char *) &Length, (char *) source.get() + index, sizeof(Length));
    index += sizeof(Length);
    allocate_space(Length);
    copy_data_from(source.get() + index, Length);
    index += Length;
}


//------------------------------------------------------------------------
// Non member operators.
//
String operator + (String &a, String &b)
{
    String	result(a, a.length() + b.length());
	
    result.append(b);
    return result;
}

int operator == (String &a, String &b)
{
    if (a.Length != b.Length)
	return 0;

    return a.compare((Object *) &b) == 0;
}

int operator != (String &a, String &b)
{
    return a.compare((Object *) &b) != 0;
}

int operator < (String &a, String &b)
{
    return a.compare((Object *) &b) == -1;
}

int operator > (String &a, String &b)
{
    return a.compare((Object *) &b) == 1;
}

int operator <= (String &a, String &b)
{
    return a.compare((Object *) &b) <= 0;
}

int operator >= (String &a, String &b)
{
    return a.compare((Object *) &b) >= 0;
}

ostream &operator << (ostream &o, String &s)
{
    o.write(s.Data, s.length());;
    return o;
}

//------------------------------------------------------------------------
// Private Methods.
//

void String::copy_data_from(char *s, int len, int dest_offset)
{
    memcpy(Data + dest_offset, s, len);
}

void String::allocate_space(int len)
{
    len++;				// In case we want to add a null.

    if (Data)
    {
	if (len > Allocated)
	    delete Data;
	else
	    return;		// No need to allocate space.
    }

    Allocated = next_power_of_2(len);
    Data = new char[Allocated];
}

void String::reallocate_space(int len)
{
    if (len + 1 > Allocated)
    {
	char	*old_data = 0;
	int		old_data_len = 0;

	if (Data)
	{
	    old_data = Data;
	    old_data_len = Length;
	    Data = 0;
	}
	allocate_space(len);
	if (old_data)
	{
	    copy_data_from(old_data, old_data_len);
	    delete [] old_data;
	}
    }
}

void String::copy(char *s, int len, int allocation_hint)
{
    Length = len;
    allocate_space(allocation_hint);
    copy_data_from(s, Length);
}

void String::debug(ostream &o)
{
    o << "Length: " << Length << " Allocated: " << Allocated <<
	" Data: " << ((void*) Data) << " '" << *this << "'\n";
}

//------------------------------------------------------------------------
// Functions private to this file.
//
int next_power_of_2(int n)
{
    //
    // There must be a faster way...
    //
    int	result = MinimumAllocationSize;
	
    while (result < n)
	result <<= 1;

    return result;
}


