//
// WordBitCompress.h
//
// BitStream: put and get bits into a buffer
//           *tagging:  add tags to keep track of the position of data 
//                      inside the bitstream for debuging purposes.
//           *freezing: saves current position. further inserts in the BitStream
//                      aren't really done. This way you can try different
//                      compression algorithms and chose the best.
//
// Compressor: BitStream with extended fuctionalities
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordBitCompress.h,v 1.7 2004/05/28 13:15:26 lha Exp $
//

#ifndef   _WordBitCompress_h
#define  _WordBitCompress_h

#include<stdio.h>
#include<stdlib.h>
#include"HtVector_int.h"
#include"HtMaxMin.h"

typedef unsigned char byte;
// ******** HtVector_byte (header)
#define GType byte
#define HtVectorGType HtVector_byte
#include "HtVectorGeneric.h"

typedef char *charptr;
// ******** HtVector_charptr (header)
#define GType charptr
#define HtVectorGType HtVector_charptr
#include "HtVectorGeneric.h"


// ******** Utility inline functions and macros

// error checking
#define FATAL_ABORT fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);abort()
#define errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);FATAL_ABORT;}
#define CHECK_MEM(p) if(!p) errr("mifluz: Out of memory!");
// max/min of 2 values
#define TMax(a,b) (((a)>(b)) ? (a) : (b))
#define TMin(a,b) (((a)<(b)) ? (a) : (b))

// compute integer log2
// == minimum number of bits needed to code value 
inline int
num_bits (unsigned int maxval)
{
  unsigned int mv = maxval;
  int nbits;
  for (nbits = 0; mv; nbits++)
  {
    mv >>= 1;
  }
  return (nbits);
}

// compute 2^x
#define pow2(x) (1<<(x))


// function declarations
char *label_str (const char *s, int n);
void show_bits (int v, int n = 16);

//  unsigned short max_v(unsigned short *vals,int n);
//  unsigned int   max_v(unsigned int   *vals,int n);
//  unsigned short min_v(unsigned short *vals,int n);
//  unsigned int   min_v(unsigned int   *vals,int n);





// **************************************************
// *************** BitStream  ***********************
// **************************************************
//  compression is done in Compressor not in BitStream
class BitStream
{
protected:

  // the buffer were the bitstream is stored
  HtVector_byte buff;

  // current bit position within the buffer
  int bitpos;

  // tags for debuging
  HtVector_int tagpos;
  HtVector_charptr tags;
  int use_tags;

  // freezing the bitstream
  HtVector_int freeze_stack;
  int freezeon;
public:
  void freeze ();
  int unfreeze ();

  // puts a bit into the bitstream
  inline void put (unsigned int v)
  {
    // SPEED CRITICAL SECTION
    if (freezeon)
    {
      bitpos++;
      return;
    }
    if (v)
    {
      buff.back () |= pow2 (bitpos & 0x07);
    }
    bitpos++;
    if (!(bitpos & 0x07))       // new byte
    {
      buff.push_back (0);
    }
  }
  inline void put (unsigned int v, const char *tag)
  {
    if (!freezeon)
    {
      add_tag (tag);
    }
    put (v);
  }

  // gets a bit from the bitstream
  inline byte get (const char *tag = (char *) NULL)
  {
    // SPEED CRITICAL SECTION
    if (check_tag (tag) == NOTOK)
    {
      errr ("BitStream::get() check_tag failed");
    }
    if (bitpos >= (buff.size () << 3))
    {
      errr ("BitStream::get reading past end of BitStream!");
    }
    byte res = buff[bitpos >> 3] & pow2 (bitpos & 0x07);
//    printf("get:res:%d bitpos:%5d/%d buff[%3d]=%x\n",res,bitpos,bitpos%8,bitpos/8,buff[bitpos/8]);
    bitpos++;
    return (res);
  }

  // get/put an integer using n bits
  void put_uint (unsigned int v, int n, const char *tag = (char *) "NOTAG");
  unsigned int get_uint (int n, const char *tag = (char *) NULL);

  // get/put n bits of data stored in vals
  void put_zone (byte * vals, int n, const char *tag);
  void get_zone (byte * vals, int n, const char *tag);

  // 
  inline void add_tag (const char *tag)
  {
    if (!use_tags || !tag || freezeon)
    {
      return;
    }
    add_tag1 (tag);
  }
  void add_tag1 (const char *tag);
  inline int check_tag (const char *tag, int pos = -1)
  {
    if (!use_tags || !tag)
    {
      return OK;
    }
    return (check_tag1 (tag, pos));
  }
  int check_tag1 (const char *tag, int pos);
  void set_use_tags ()
  {
    use_tags = 1;
  }
  int find_tag (const char *tag);
  int find_tag (int pos, int posaftertag = 1);

  void show_bits (int a, int n);
  void show (int a = 0, int n = -1);

  // position accesors
  int size ()
  {
    return (bitpos);
  }
  int buffsize ()
  {
    return (buff.size ());
  }

  // get a copy of the buffer
  byte *get_data ();
  // set the buffer from outside data (current buffer must be empty)
  void set_data (const byte * nbuff, int nbits);

  // use this for reading a BitStream after you have written in it 
  // (generally for debuging)
  void rewind ()
  {
    bitpos = 0;
  }

  ~BitStream ()
  {
    int i;
    for (i = 0; i < tags.size (); i++)
    {
      free (tags[i]);
    }
  }
  BitStream (int size0)
  {
    buff.reserve ((size0 + 7) / 8);
    init ();
  }
  BitStream ()
  {
    init ();
  }
private:
  void init ()
  {
    bitpos = 0;
    buff.push_back (0);
    freezeon = 0;
    use_tags = 0;
  }
};


// **************************************************
// *************** Compressor ***********************
// **************************************************

// Constants used by Compressor
// number of bits to code the number of values in an array 
#define NBITS_NVALS 16
// number of bits to code the values in an unsigned int array (=sizeof(unsigned int))
#define NBITS_VAL 32
// number of bits to code he number of bits used by an unsigned int value
#define NBITS_NBITS_VAL  5
// number of bits to code the number of bits used by a byte value
#define NBITS_NBITS_CHARVAL 4

class Compressor:public BitStream
{
public:
  int verbose;
  // get/put an integer using a variable number of bits
  void put_uint_vl (unsigned int v, int maxn, const char *tag =
                    (char *) "NOTAG");
  unsigned int get_uint_vl (int maxn, const char *tag = (char *) NULL);

  // get/put an integer checking for an expected value
  void put_uint_ex (unsigned int v, unsigned int ex, int maxn,
                    const char *tag = (char *) "NOTAG")
  {
    if (v == ex)
    {
      put (1, tag);
    }
    else
    {
      put (0, tag);
      put_uint (v, maxn, (char *) NULL);
    }
  }
  unsigned int get_uint_ex (unsigned int ex, int maxn, const char *tag =
                            (char *) NULL)
  {
    if (get (tag))
    {
      return ex;
    }
    else
    {
      return get_uint (maxn, (char *) NULL);
    }
  }


  // compress/decompress an array of unsigned ints (choosing best method)
  int put_vals (unsigned int *vals, int n, const char *tag);
  int get_vals (unsigned int **pres, const char *tag = (char *) "BADTAG!");

  // compress/decompress an array of bytes (very simple)
  int put_fixedbitl (byte * vals, int n, const char *tag);
  int get_fixedbitl (byte ** pres, const char *tag = (char *) "BADTAG!");

  // compress/decompress an array of unsigned ints (very simple)
  void get_fixedbitl (unsigned int *res, int n);
  void put_fixedbitl (unsigned int *vals, int n);

  // compress/decompress an array of unsigned ints (sophisticated)
  void get_decr (unsigned int *res, int n);
  void put_decr (unsigned int *vals, int n);

Compressor ():BitStream ()
  {
    verbose = 0;
  }
  Compressor (int size0):BitStream (size0)
  {
    verbose = 0;
  }

};



#endif
