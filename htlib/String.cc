//
// Implementation of String class
//
// $Id: String.cc,v 1.21 1999/08/02 09:04:10 angus Exp $
//
#if RELEASE
static char	RCSid[] = "$Id: String.cc,v 1.21 1999/08/02 09:04:10 angus Exp $";
#endif


#include "htString.h"

#include <unistd.h>
#include <stream.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "Object.h"


const int MinimumAllocationSize = 4;	// Should be power of two.

#ifdef NOINLINE
String::String()
{
    Length = 0;
    Allocated = 0;
//  Ok it's a hack,  but we are in big league here
//  70 000 000 call
//  Data = 0;
}
#endif

String::String(int init)
{
    Length = 0;
    Allocated = init;
    Data = new char[init];
}

String::String(char *s)
{
    Allocated = 0;
    Length = 0;

    int	len;
    if (s)
      {
	len = strlen(s);
	copy(s, len, len);
      }
}

String::String(char *s, int len)
{
    Allocated = 0;
    Length = 0;
    if (s && len != 0)
	copy(s, len, len);
}

String::String(const String &s)
{
    Allocated = 0;
    Length = 0;
    if (s.length() != 0)
      copy(s.Data, s.length(), s.length());
}

//
// This can be used for performance reasons if it is known the
// String will need to grow.
//
String::String(const String &s, int allocation_hint)
{
    Allocated = 0;
    Length = 0;
    if (s.length() != 0)
      {
	if (allocation_hint < s.length())
	  allocation_hint = s.length();
	copy(s.Data, s.length(), allocation_hint);
      }
}

String::~String()
{
    if (Allocated)
	delete [] Data;
}

void String::operator = (const String &s)
{
    allocate_space(s.length());
    Length = s.length();
    copy_data_from(s.Data, Length);
}

void String::operator = (char *s)
{
    if (s)
    {
      int len = strlen(s);
	allocate_fix_space(len);
	Length = len;
	copy_data_from(s, Length);	
    }
    else
	Length = 0;
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
	
    append(s,strlen(s));
}

void String::append(char *s, int slen)
{
    if (!s || !slen)
	return;

//    if ( slen == 1 ) 
//    {
//        append(*s);
//        return;
//    }
    int	new_len = Length + slen;

    if (new_len + 1 > Allocated)
    reallocate_space(new_len);
    copy_data_from(s, slen, Length);
    Length = new_len;
}

void String::append(char ch)
{
    int new_len = Length +1;
    if (new_len + 1 > Allocated)
    	reallocate_space(new_len);
    Data[Length] = ch;
    Length = new_len;
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
static char	*null = "";
    if (!Allocated)
	return null;
    Data[Length] = '\0';	// We always leave room for this.
    return Data;
}

char *String::new_char() const
{
    char	*r;
    if (!Allocated)
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
    char	*c;    
    //
    // Set the first char after string end to zero to prevent finding
    // substrings including symbols after actual end of string
    //
    if (!Allocated)
	return -1;
    Data[Length] = '\0';
    
    /* OLD CODE: for (i = 0; i < Length; i++) */
#ifdef HAVE_STRSTR
    if ((c = strstr(Data, str)) != NULL)
	return(c -Data);
#else
    int		len = strlen(str);
    int		i;
    for (i = 0; i <= Length-len; i++)
    {
	if (strncmp(&Data[i], str, len) == 0)
	    return i;
    }
#endif
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

int String::lastIndexOf(char ch)
{
    return lastIndexOf(ch, Length - 1);
}
#ifdef NOINLINE
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
#endif

String &String::operator << (int i)
{
    char	str[20];
    sprintf(str, "%d", i);
    append(str);
    return *this;
}

String &String::operator << (unsigned int i)
{
    char	str[20];
    sprintf(str, "%u", i);
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
	
    if (Allocated && Length)
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

char	String::Nth (int n)
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
    return new String(*this);
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
	while (Length > 0 && Data[Length - 1] == ch)
	    Length--;
    return *this;
}


String &String::chop(char *str)
{
	while (Length > 0 && strchr(str, Data[Length - 1]))
	    Length--;
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
    allocate_fix_space(Length);
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

    if (len <= Allocated)
      return;

    if (Allocated)
      delete [] Data;

    Allocated = MinimumAllocationSize;
    while (Allocated < len)
      Allocated <<= 1;

    Data = new char[Allocated];
}

void String::allocate_fix_space(int len)
{
    len++;				// In case we want to add a null.

    if (len <= Allocated)
      return;

    if (Allocated)
      delete [] Data;

    Allocated = len;
    if (Allocated < MinimumAllocationSize)
      Allocated = MinimumAllocationSize;
    Data = new char[Allocated];
}

void String::reallocate_space(int len)
{
	char	*old_data = 0;
	int	 old_data_len = 0;

    if (Allocated)
	{
	    old_data = Data;
	    old_data_len = Length;
	    Allocated = 0;
	}
    allocate_space(len);
    if (old_data)
      {
	copy_data_from(old_data, old_data_len);
	delete [] old_data;
      }
}

void String::copy(char *s, int len, int allocation_hint)
{
  if (len == 0 || allocation_hint == 0)
    return;         // We're not actually copying anything!
  allocate_fix_space(allocation_hint);
  Length = len;
  copy_data_from(s, len);
}

void String::debug(ostream &o)
{
    o << "Length: " << Length << " Allocated: " << Allocated <<
	" Data: " << ((void*) Data) << " '" << *this << "'\n";
}


