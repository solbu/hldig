//
// HtPack.cc
//
// HtPack: Compress and uncompress data in e.g. simple structures.
//	   The structure must have the layout defined in the ABI;
//	   the layout the compiler generates.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtPack.cc,v 1.1.2.1 2006/09/25 23:50:30 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtPack.h"

#include <ctype.h>
#include <stdlib.h>

// For the moment, these formats are accepted:
// "i" native int, with most compressed value 0
// "u" unsigned int, with most compressed value 0
// "c" unsigned int, with most compressed value 1.
//
//  If someone adds other formats (and uses them), please note
// that structure padding may give surprising effects on some
// (most) platforms, for example if you try to unpack a
// structure with the imagined signature "isi" (int, short, int).
//  You will want to solve that portably.
//
// Compression is done to 2 bits description (overhead) each,
// plus variable-sized data.
//  Theoretically, different formats can use different number of
// bits in the description with a few changes.
//  The description is located in a byte before every four
// "fields".
String
htPack(const char format[], const char *data)
{
  const char *s = format;

  // We insert the encodings by number, rather than shifting and
  // inserting at the "bottom".	 This should make it faster for
  // decoding, which presumably is more important than the speed
  // of encoding.
  int code_no = 0;

  // Make a wild guess that we will compress some ordinary sized
  // struct.  This guess only has speed effects.
  String compressed(60);

  // Accumulated codes.
  unsigned int description = 0;

  // Store the encoding here.  We cannot use a char *, as the
  // string may be reallocated and moved.
  int code_index = 0;

  // Make place for the first codes.
  compressed << '\0';

  // Format string loop.
  while (*s)
  {
    int fchar = *s++;
    int n;

    if (isdigit(*s))
    {
      char* t;
      n = strtol(s, &t, 10);
      s = t;
    }
    else
      n = 1;

    // Loop over N in e.g. "iN" (default 1).
    while (n--)
    {
      // Format character handling.
      switch (fchar)
      {
	case 'c':
	{
	  // We compress an unsigned int with the most common
	  // value 1 as this:
	  // 00 - value is 1.
	  // 01 - value fits in unsigned char - appended.
	  // 10 - value fits in unsigned short - appended.
	  // 11 - just plain unsigned int - appended (you lose).
	  unsigned int value;

	  // Initialize, but allow disalignment.
	  memcpy(&value, data, sizeof value);
	  data += sizeof(unsigned int);

	  int mycode;
	  if (value == 1)
	  {
	    mycode = 0;
	  }
	  else
	  {
	    unsigned char charvalue = (unsigned char) value;
	    unsigned short shortvalue = (unsigned short) value;
	    if (value == charvalue)
	    {
	      mycode = 1;
	      compressed << charvalue;
	    }
	    else if (value == shortvalue)
	    {
	      mycode = 2;
	      compressed.append((char *) &shortvalue, sizeof shortvalue);
	    }
	    else
	    {
	      mycode = 3;
	      compressed.append((char *) &value, sizeof value);
	    }
	  }

	  description |= mycode << (2*code_no++);
	}
	break;

	case 'i':
	{
	  // We compress a (signed) int as follows:
	  // 00 - value is 0.
	  // 01 - value fits in char - appended.
	  // 10 - value fits in short - appended.
	  // 11 - just plain int - appended (you lose).
	  int value;

	  // Initialize, but allow disalignment.
	  memcpy(&value, data, sizeof value);
	  data += sizeof(int);

	  int mycode;
	  if (value == 0)
	  {
	    mycode = 0;
	  }
	  else
	  {
	    char charvalue = char(value);
	    short shortvalue = short(value);
	    if (value == charvalue)
	    {
	      mycode = 1;
	      compressed << charvalue;
	    }
	    else if (value == shortvalue)
	    {
	      mycode = 2;
	      compressed.append((char *) &shortvalue, sizeof shortvalue);
	    }
	    else
	    {
	      mycode = 3;
	      compressed.append((char *) &value, sizeof value);
	    }
	  }

	  description |= mycode << (2*code_no++);
	}
	break;

	case 'u':
	{
	  // We compress an unsigned int like an int:
	  // 00 - value is 0.
	  // 01 - value fits in unsigned char - appended.
	  // 10 - value fits in unsigned short - appended.
	  // 11 - just plain unsigned int - appended (you lose).
	  unsigned int value;

	  // Initialize, but allow disalignment.
	  memcpy(&value, data, sizeof value);
	  data += sizeof(unsigned int);

	  int mycode;
	  if (value == 0)
	  {
	    mycode = 0;
	  }
	  else
	  {
	    unsigned char charvalue = (unsigned char) value;
	    unsigned short shortvalue = (unsigned short) value;
	    if (value == charvalue)
	    {
	      mycode = 1;
	      compressed << charvalue;
	    }
	    else if (value == shortvalue)
	    {
	      mycode = 2;
	      compressed.append((char *) &shortvalue, sizeof shortvalue);
	    }
	    else
	    {
	      mycode = 3;
	      compressed.append((char *) &value, sizeof value);
	    }
	  }

	  description |= mycode << (2*code_no++);
	}
	break;

	default:
#ifndef NOSTREAM
#ifdef DEBUG
	  if (1)
	    cerr << "Invalid char \'" << char(fchar)
		 << "\' in pack format \"" << format << "\""
		 << endl;
	  return "";
#endif
#endif
	  ; // Must always have a statement after a label.
      }

      // Assuming 8-bit chars here.  Flush encodings after 4 (2 bits
      // each) or when the code-string is consumed.
      if (code_no == 4 || (n == 0 && *s == 0))
      {
	char *codepos = compressed.get() + code_index;

	*codepos = description;
	description = 0;
	code_no = 0;

	if (n || *s)
	{
	  // If more data to be encoded, then we need a new place to
	  // store the encodings.
	  code_index = compressed.length();
	  compressed << '\0';
	}
      }
    }
  }

  return compressed;
}


// Reverse the effect of htPack.
String
htUnpack(const char format[], const char *data)
{
  const char *s = format;

  // The description needs to be renewed immediately.
  unsigned int description = 1;

  // Make a wild guess about that we decompress to some ordinary
  // sized struct and assume the cost of allocation some extra
  // memory is much less than the cost of allocating more.
  // This guess only has speed effects.
  String decompressed(60);

  // Format string loop.
  while (*s)
  {
    int fchar = *s++;
    int n;

    if (isdigit(*s))
    {
      char* t;
      n = strtol(s, &t, 10);
      s = t;
    }
    else
      n = 1;

    // Loop over N in e.g. "iN" (default 1).
    while (n--)
    {
      // Time to renew description?
      if (description == 1)
	description = 256 | *data++;

      // Format character handling.
      switch (fchar)
      {
	case 'c':
	{
	  // An unsigned int with the most common value 1 is
          // compressed as follows:
	  // 00 - value is 1.
	  // 01 - value fits in unsigned char - appended.
	  // 10 - value fits in unsigned short - appended.
	  // 11 - just plain unsigned int - appended (you lose).
	  unsigned int value;

	  switch (description & 3)
	  {
	    case 0:
	    value = 1;
	    break;

	    case 1:
	    {
	      unsigned char charvalue;
	      memcpy(&charvalue, data, sizeof charvalue);
	      value = charvalue;
	      data++;
	    }
	    break;

	    case 2:
	    {
	      unsigned short int shortvalue;
	      memcpy(&shortvalue, data, sizeof shortvalue);
	      value = shortvalue;
	      data += sizeof shortvalue;
	    }
	    break;

	    case 3:
	    {
	      memcpy(&value, data, sizeof value);
	      data += sizeof value;
	    }
	    break;
	  }
	  decompressed.append((char *) &value, sizeof value);
	}
	break;

	case 'i':
	{
	  // A (signed) int is compressed as follows:
	  // 00 - value is 0.
	  // 01 - value fits in char - appended.
	  // 10 - value fits in short - appended.
	  // 11 - just plain int - appended (you lose).
	  int value;

	  switch (description & 3)
	  {
	    case 0:
	    value = 0;
	    break;

	    case 1:
	    {
	      char charvalue;
	      memcpy(&charvalue, data, sizeof charvalue);
	      value = charvalue;
	      data++;
	    }
	    break;

	    case 2:
	    {
	      short int shortvalue;
	      memcpy(&shortvalue, data, sizeof shortvalue);
	      value = shortvalue;
	      data += sizeof shortvalue;
	    }
	    break;

	    case 3:
	    {
	      memcpy(&value, data, sizeof value);
	      data += sizeof value;
	    }
	    break;
	  }
	  decompressed.append((char *) &value, sizeof value);
	}
	break;

	case 'u':
	{
	  // An unsigned int is compressed as follows:
	  // 00 - value is 0.
	  // 01 - value fits in unsigned char - appended.
	  // 10 - value fits in unsigned short - appended.
	  // 11 - just plain unsigned int - appended (you lose).
	  unsigned int value;

	  switch (description & 3)
	  {
	    case 0:
	    value = 0;
	    break;

	    case 1:
	    {
	      unsigned char charvalue;
	      memcpy(&charvalue, data, sizeof charvalue);
	      value = charvalue;
	      data++;
	    }
	    break;

	    case 2:
	    {
	      unsigned short int shortvalue;
	      memcpy(&shortvalue, data, sizeof shortvalue);
	      value = shortvalue;
	      data += sizeof shortvalue;
	    }
	    break;

	    case 3:
	    {
	      memcpy(&value, data, sizeof value);
	      data += sizeof value;
	    }
	    break;
	  }
	  decompressed.append((char *) &value, sizeof value);
	}
	break;

	default:
#ifndef NOSTREAM
#ifdef DEBUG
	  if (1)
	    cerr << "Invalid char \'" << char(fchar)
		 << "\' in unpack format \"" << format << "\""
		 << endl;
	  return "";
#endif
#endif
	  ; // Must always have a statement after a label.
      }

      description >>= 2;
    }
  }

  return decompressed;
}

// End of HtPack.cc
