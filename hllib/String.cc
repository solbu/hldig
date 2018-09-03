//
// String.cc
//
// String: (interface in htString.h) Just Another String class.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: String.cc,v 1.40 2004/05/28 13:15:21 lha Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */


#include "htString.h"
#include "Object.h"

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


const int MinimumAllocationSize = 4;    // Should be power of two.

#ifdef NOINLINE
String::String ()
{
  Length = Allocated = 0;
  Data = 0;
}
#endif

String::String (int init)
{
  Length = 0;
  Allocated = init >= MinimumAllocationSize ? init : MinimumAllocationSize;
  Data = new char[Allocated];
}

String::String (const char *s)
{
  Allocated = Length = 0;
  Data = 0;

  int len;
  if (s)
  {
    len = strlen (s);
    copy (s, len, len);
  }
}

String::String (const char *s, int len)
{
  Allocated = Length = 0;
  Data = 0;
  if (s && len > 0)
    copy (s, len, len);
}

String::String (const String & s)
{
  Allocated = Length = 0;
  Data = 0;

  if (s.length () > 0)
    copy (s.Data, s.length (), s.length ());
}

//
// This can be used for performance reasons if it is known the
// String will need to grow.
//
String::String (const String & s, int allocation_hint)
{
  Allocated = Length = 0;
  Data = 0;

  if (s.length () != 0)
  {
    if (allocation_hint < s.length ())
      allocation_hint = s.length ();
    copy (s.Data, s.length (), allocation_hint);
  }
}

String::~String ()
{
  if (Allocated)
    delete[]Data;
}

void
String::operator = (const String & s)
{
  if (s.length () > 0)
  {
    allocate_space (s.length ());
    Length = s.length ();
    copy_data_from (s.Data, Length);
  }
  else
  {
    Length = 0;
  }
}

void
String::operator = (const char *s)
{
  if (s)
  {
    int len = strlen (s);
    allocate_fix_space (len);
    Length = len;
    copy_data_from (s, Length);
  }
  else
    Length = 0;
}

void
String::append (const String & s)
{
  if (s.length () == 0)
    return;
  int new_len = Length + s.length ();

  reallocate_space (new_len);
  copy_data_from (s.Data, s.length (), Length);
  Length = new_len;
}

void
String::append (const char *s)
{
  if (!s)
    return;

  append (s, strlen (s));
}

void
String::append (const char *s, int slen)
{
  if (!s || !slen)
    return;

//    if ( slen == 1 )
//    {
//        append(*s);
//        return;
//    }
  int new_len = Length + slen;

  if (new_len + 1 > Allocated)
    reallocate_space (new_len);
  copy_data_from (s, slen, Length);
  Length = new_len;
}

void
String::append (char ch)
{
  int new_len = Length + 1;
  if (new_len + 1 > Allocated)
    reallocate_space (new_len);
  Data[Length] = ch;
  Length = new_len;
}

int
String::compare (const String & obj) const
{
  int len;
  int result;
  const char *p1 = Data;
  const char *p2 = obj.Data;

  len = Length;
  result = 0;

  if (Length > obj.Length)
  {
    result = 1;
    len = obj.Length;
  }
  else if (Length < obj.Length)
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

int
String::nocase_compare (const String & s) const
{
  const char *p1 = get ();
  const char *p2 = s.get ();

  return mystrcasecmp (p1, p2);
}

int
String::Write (int fd) const
{
  int left = Length;
  char *wptr = Data;

  while (left)
  {
    int result = write (fd, wptr, left);

    if (result < 0)
      return result;

    left -= result;
    wptr += result;
  }
  return left;
}

const char *
String::get () const
{
  static const char *null = "";
  if (!Allocated)
    return null;
  Data[Length] = '\0';          // We always leave room for this.
  return Data;
}

char *
String::get ()
{
  if (!Allocated)
  {
    static char null[] = { "" };
    return null;
  }
  Data[Length] = '\0';          // We always leave room for this.
  return Data;
}

char *
String::new_char () const
{
  char *r;
  if (!Allocated)
  {
    r = new char[1];
    *r = '\0';
    return r;
  }
  Data[Length] = '\0';          // We always leave room for this.
  r = new char[Length + 1];
  strcpy (r, Data);
  return r;
}


int
String::as_integer (int def) const
{
  if (Length <= 0)
    return def;
  Data[Length] = '\0';
  return atoi (Data);
}

double
String::as_double (double def) const
{
  if (Length <= 0)
    return def;
  Data[Length] = '\0';
  return atof (Data);
}

String
String::sub (int start, int len) const
{
  if (start > Length)
    return 0;

  if (len > Length - start)
    len = Length - start;

  return String (Data + start, len);
}

String
String::sub (int start) const
{
  return sub (start, Length - start);
}

int
String::indexOf (const char *str) const
{
  char *c;
  //
  // Set the first char after string end to zero to prevent finding
  // substrings including symbols after actual end of string
  //
  if (!Allocated)
    return -1;
  Data[Length] = '\0';

  /* OLD CODE: for (i = 0; i < Length; i++) */
#ifdef HAVE_STRSTR
  if ((c = strstr (Data, str)) != NULL)
    return (c - Data);
#else
  int len = strlen (str);
  int i;
  for (i = 0; i <= Length - len; i++)
  {
    if (strncmp (&Data[i], str, len) == 0)
      return i;
  }
#endif
  return -1;
}

int
String::indexOf (char ch) const
{
  int i;
  for (i = 0; i < Length; i++)
  {
    if (Data[i] == ch)
      return i;
  }
  return -1;
}

int
String::indexOf (char ch, int pos) const
{
  if (pos >= Length)
    return -1;
  for (int i = pos; i < Length; i++)
  {
    if (Data[i] == ch)
      return i;
  }
  return -1;
}

int
String::lastIndexOf (char ch, int pos) const
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

int
String::lastIndexOf (char ch) const
{
  return lastIndexOf (ch, Length - 1);
}

#ifdef NOINLINE
String & String::operator << (const char *str)
{
  append (str);
  return *this;
}

String & String::operator << (char ch)
{
  append (&ch, 1);
  return *this;
}
#endif

String & String::operator << (int i)
{
  char
    str[20];
  sprintf (str, "%d", i);
  append (str);
  return *this;
}

String & String::operator << (unsigned int i)
{
  char
    str[20];
  sprintf (str, "%u", i);
  append (str);
  return *this;
}

String & String::operator << (long l)
{
  char
    str[20];
  sprintf (str, "%ld", l);
  append (str);
  return *this;
}

String & String::operator << (const String & s)
{
  append (s.get (), s.length ());
  return *this;
}

char
String::operator >> (char c)
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

int
String::lowercase ()
{
  int converted = 0;
  for (int i = 0; i < Length; i++)
  {
    if (isupper ((unsigned char) Data[i]))
    {
      Data[i] = tolower ((unsigned char) Data[i]);
      converted++;
    }
  }
  return converted;
}


int
String::uppercase ()
{
  int converted = 0;
  for (int i = 0; i < Length; i++)
  {
    if (islower ((unsigned char) Data[i]))
    {
      Data[i] = toupper ((unsigned char) Data[i]);
      converted++;
    }
  }
  return converted;
}


void
String::replace (char c1, char c2)
{
  for (int i = 0; i < Length; i++)
    if (Data[i] == c1)
      Data[i] = c2;
}


int
String::remove (const char *chars)
{
  if (Length <= 0)
    return 0;

  char *good, *bad;
  int skipped = 0;

  good = bad = Data;
  for (int i = 0; i < Length; i++)
  {
    if (strchr (chars, *bad))
      skipped++;
    else
      *good++ = *bad;
    bad++;
  }
  Length -= skipped;

  return skipped;
}

String & String::chop (int n)
{
  Length -= n;
  if (Length < 0)
    Length = 0;
  return *this;
}


String & String::chop (char ch)
{
  while (Length > 0 && Data[Length - 1] == ch)
    Length--;
  return *this;
}


String & String::chop (const char *str)
{
  while (Length > 0 && strchr (str, Data[Length - 1]))
    Length--;
  return *this;
}


void
String::Serialize (String & dest)
{
  dest.append ((char *) &Length, sizeof (Length));
  dest.append (get (), Length);
}


void
String::Deserialize (String & source, int &index)
{
  memcpy ((char *) &Length, (char *) source.get () + index, sizeof (Length));
  index += sizeof (Length);
  allocate_fix_space (Length);
  copy_data_from (source.get () + index, Length);
  index += Length;
}


//------------------------------------------------------------------------
// Non member operators.
//
String
operator + (const String & a, const String & b)
{
  String result (a, a.length () + b.length ());

  result.append (b);
  return result;
}

int
operator == (const String & a, const String & b)
{
  if (a.Length != b.Length)
    return 0;

  return a.compare (b) == 0;
}

int
operator != (const String & a, const String & b)
{
  return a.compare (b) != 0;
}

int
operator < (const String & a, const String & b)
{
  return a.compare (b) == -1;
}

int
operator > (const String & a, const String & b)
{
  return a.compare (b) == 1;
}

int
operator <= (const String & a, const String & b)
{
  return a.compare (b) <= 0;
}

int
operator >= (const String & a, const String & b)
{
  return a.compare (b) >= 0;
}

#ifndef NOSTREAM
ostream & operator << (ostream & o, const String & s)
{
  o.write (s.Data, s.length ());
  return o;
}
#endif /* NOSTREAM */

//------------------------------------------------------------------------
// Private Methods.
//

void
String::copy_data_from (const char *s, int len, int dest_offset)
{
  memcpy (Data + dest_offset, s, len);
}

void
String::allocate_space (int len)
{
  len++;                        // In case we want to add a null.

  if (len <= Allocated)
    return;

  if (Allocated)
    delete[]Data;

  Allocated = MinimumAllocationSize;
  while (Allocated < len)
    Allocated <<= 1;

  Data = new char[Allocated];
}

void
String::allocate_fix_space (int len)
{
  len++;                        // In case we want to add a null.

  if (len <= Allocated)
    return;

  if (Allocated)
    delete[]Data;

  Allocated = len;
  if (Allocated < MinimumAllocationSize)
    Allocated = MinimumAllocationSize;
  Data = new char[Allocated];
}

void
String::reallocate_space (int len)
{
  char *old_data = 0;
  int old_data_len = 0;

  if (Allocated)
  {
    old_data = Data;
    old_data_len = Length;
    Allocated = 0;
  }
  allocate_space (len);
  if (old_data)
  {
    copy_data_from (old_data, old_data_len);
    delete[]old_data;
  }
}

void
String::copy (const char *s, int len, int allocation_hint)
{
  if (len == 0 || allocation_hint == 0)
    return;                     // We're not actually copying anything!
  allocate_fix_space (allocation_hint);
  Length = len;
  copy_data_from (s, len);
}

#ifndef NOSTREAM
void
String::debug (ostream & o)
{
  o << "Length: " << Length << " Allocated: " << Allocated <<
    " Data: " << ((void *) Data) << " '" << *this << "'\n";
}
#endif /* NOSTREAM */

int
String::readLine (FILE * in)
{
  Length = 0;
  allocate_fix_space (2048);

  while (fgets (Data + Length, Allocated - Length, in))
  {
    Length += strlen (Data + Length);
    if (Length == 0)
      continue;
    if (Data[Length - 1] == '\n')
    {
      //
      // A full line has been read.  Return it.
      //
      chop ('\n');
      return 1;
    }
    if (Allocated > Length + 1)
    {
      //
      // Not all available space filled. Probably EOF?
      //
      continue;
    }
    //
    // Only a partial line was read. Increase available space in
    // string and read some more.
    //
    reallocate_space (Allocated << 1);
  }
  chop ('\n');

  return Length > 0;
}

#ifndef NOSTREAM
istream & operator >> (istream & in, String & line)
{
  line.Length = 0;
  line.allocate_fix_space (2048);

  for (;;)
  {
    in.clear ();
    in.getline (line.Data + line.Length, line.Allocated - line.Length);
    line.Length += strlen (line.Data + line.Length);
    // if read whole line, or eof, or read fewer chars than the max...
    if (!in.fail () || in.eof () || line.Length + 1 < line.Allocated)
      break;
    //
    // Only a partial line was read. Increase available space in
    // string and read some more.
    //
    line.reallocate_space (line.Allocated << 1);
  }

  return in;
}
#endif /* NOSTREAM */
