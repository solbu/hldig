//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//

#ifndef _ber_h
#define _ber_h

#include <stdio.h>
#include <errno.h>

typedef unsigned int ber_t;

#define BER_MAX_BYTES (sizeof(ber_t) + 1)

inline int ber_buf2value(const unsigned char* buf, int buf_len, ber_t& result) {
  result = 0;
  unsigned int bits = 0;
  int length = 1;
  while(*buf & 0x80) {
    if(bits > sizeof(ber_t) * 8) return EINVAL;
    result |= (*buf & 0x7f) << bits;
    bits += 7;
    buf++;
    length++;
    if(length > buf_len) return EINVAL;
  }
  result |= (*buf & 0x7f) << bits;

  return length;
}

inline int ber_file2value(FILE* fp, ber_t& result) {
  result = 0;
  unsigned int bits = 0;
  int c;
  int length = 1;
  while((c = fgetc(fp)) != EOF && (c & 0x80)) {
    if(bits > sizeof(ber_t) * 8) return EINVAL;
    result |= (c & 0x7f) << bits;
    bits += 7;
    length++;
  }

  if(c == EOF) return EINVAL;

  result |= (c & 0x7f) << bits;

  return length;
}

inline int ber_value2buf(unsigned char* buf, int buf_len, ber_t value)
{
  if(buf_len <= 0) return EINVAL;
  int buf_idx = 0;
  buf[buf_idx++] = (value & 0x7f);
  while(value >>= 7) {
    if(buf_idx >= buf_len) return EINVAL;
    buf[buf_idx - 1] |= 0x80;
    buf[buf_idx++] = (value & 0x7f);
  }
  return buf_idx;
}

inline int ber_value2file(FILE* fp, ber_t value)
{
  int length = 1;
  unsigned char current;
  current = (value & 0x7f);
  while(value >>= 7) {
    current |= 0x80;
    if(fputc(current, fp) == EOF) return EINVAL;
    current = (value & 0x7f);
    length++;
  }
  
  if(fputc(current, fp) == EOF) return EINVAL;

  return length;
}

#endif /* _ber_h */
