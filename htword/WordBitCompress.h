//
// WordBitCompress.h
//
// NAME
//
// Read and write objects in a bit stream, possibly compressing them.
//
// SYNOPSIS
//
// #include <WordBitStream.h>
//
// unsigned int value = 200;
// WordBitCompress stream;
// stream.PutUint(value, 8);
// stream.Rewind();
// stream.GetUint(value);
//
// DESCRIPTION
//
// A <b>WordBitCompress</b> object is a variable size string with methods
// to write and read numerical objects using a controlled amount of bits.
// Some methods implement compression such as custom Shannon-Fano or 
// static compression similar to Golomb or Gamma. 
//
// It is not possible to read a stream while writing into it and vice 
// versa. Another limitation is that the largest number is of type unsigned
// int. 
//
// For examples of use, check the <b>WordDBCompress</b> implementation. 
//
// 
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordBitCompress.h,v 1.1.2.10 2000/09/14 03:13:25 ghutchis Exp $
//

#ifndef _WordBitCompress_h
#define _WordBitCompress_h

#include<stdio.h>
#include<stdlib.h>

class WordBitStream {
public:
  inline WordBitStream(unsigned int nbuff_size) {
    Init();
    Allocate(nbuff_size);
  }
  inline WordBitStream() {
    Init();
  }
  inline ~WordBitStream() {
    free(buff);
  }

  //
  // Build and initialize stream
  //
  inline void Init() {
    buff_size = 1024;
    buff = (unsigned char*)malloc(buff_size);
    Clear();
  }

  //
  // Reset stream to empty state
  //
  inline void Clear() {
    bitpos = 0;
    buff_length = 1;
    buff_idx = 0;
    buff[buff_idx] = '\0';
    freezeon = 0;
    freeze_bitcount = 0;
  }

  inline void Allocate(unsigned int nbuff_size) {
    buff_size = nbuff_size;
    buff = (unsigned char*)realloc(buff, buff_size);
  }

  //
  // Make sure the buffer is large enough to safely access the
  // byte located at buff[index]
  //
  inline void Check(int index) {
    while(index >= buff_size)
      Allocate(buff_size * 2);
  }

  //
  // Advance bitpos by adding incr to it, take care of space allocation.
  //
  inline void BitposIncr(int incr) {
    bitpos += incr;
    if(!(bitpos & 0x07)) {
      Check(++buff_idx);
      buff[buff_idx] = 0;
      buff_length++;
    }
  }

  //
  // Append a bit : 0 if v null, 1 otherwise
  //
  inline void Put(unsigned int v) {
    if(freezeon) {
      freeze_bitcount++;
      return;
    }

    if(v) buff[buff_idx] |= 1 << (bitpos & 0x07);
    BitposIncr(1);
  }
  //
  // Return 0 if current bit in stream is not set, and not 0 if it is set. The
  // bit position is incremented.
  //
  inline unsigned char Get() {
    if(bitpos >= (buff_length << 3)) {
      fprintf(stderr, "BitStream::get reading past end of BitStream.\n");
    }
    
    unsigned char res = buff[bitpos >> 3] & (1 << (bitpos & 0x07));
    bitpos++;
    return res;
  }

  //
  // Put in stream the lowest <b>n</b> bits from the value <b>v</b>
  //
  void PutUint(unsigned int v, int n);
  //
  // Return <b>n</b> bits from the stream.
  //
  unsigned int GetUint(int n);

  //
  // Put in stream <b>n</b> bits from the array <b>vals</b>, each char
  // considered as a storage for 8 bits.
  //
  void PutZone(unsigned char *vals, int n);
  //
  // Get from stream <b>n</b> bits and move them to the array <b>vals</b>.
  // The size of <b>vals</b> must be at least <b>n</b>/8 long.
  //
  void GetZone(unsigned char *vals, int n);

  //
  // Return the length of the stream, in bits.
  //
  inline int Length() { return freezeon ? freeze_bitcount : bitpos; }
  //
  // Return the length of the stream, in bytes.
  //
  inline int BuffLength() { return buff_length; }
  //
  // Return a pointer to the beginning of the stream.
  //
  inline unsigned char* Buff() { return buff; }
  //
  // Return a pointer to a copy of the stream allocated with malloc.
  //
  inline unsigned char* BuffCopy() {
    unsigned char* copy = (unsigned char*)malloc(buff_length);
    memcpy(copy, buff, buff_length);
    return copy;
  }
  //
  // Copy <b>nbuff_length</b> bytes from <b>nbuff</b> into the stream
  //
  inline void BuffSet(const unsigned char* nbuff, int nbuff_length) {
    bitpos = 0;
    buff_length = nbuff_length;
    buff_idx = buff_length - 1;
    Check(buff_length);
    memcpy(buff, nbuff, buff_length);
  }
  //
  // Move the stream cursor to the beginning of the stream.
  //
  void Rewind() { bitpos = 0; }

  //
  // Stop insertion into the stream. Only the stream cursor is updated
  // to accurately reflect the size of the stream. All the Get* and Put* 
  // methods behaviour is modified.
  //
  void Freeze() {
    if(freezeon) {
      fprintf(stderr, "WordBitCompress::Freeze: recursive call not permitted\n");
    }
    freeze_bitcount = 0;
    freezeon = 1;
  }
  //
  // Enable insertion into the stream, this is the normal behaviour. See
  // the <b>Freeze</b> method.
  //
  void UnFreeze() {
    freezeon = 0;
  }
      
private:

  // the buffer were the bitstream is stored
  unsigned char* buff;
  int buff_length;
  int buff_size;
  int buff_idx;

  // current bit position within the buffer
  int bitpos;

  // freezing the bitstream
  int freeze_bitcount;
  int freezeon;
};

// Constants used by WordBitCompress
// number of bits to code the number of values in an array 
#define WORD_CMPR_NBITS_NVALS 16

//
// Number of bits encoding the log2 of 32 bits value
// 5 == log2(32) == log2(log2(0xffffffff))
//
#define WORD_CMPR_LOG32_BITS	5

//
// Number of bits encoding the log2 of 8 bits value
// 3 == log2(8) == log2(log2(0xff))
// (currently set to 4 for unknown reasons, never tried with
//  3. The fact that the above LOG32_BITS = 5 apparenlty works
//  is not a good hint to guess that LOG8_BITS = 3 would work
//  since the coding functions refuse values with bit 32 set
//  so the number of bits really encoded is really 31. To
//  make sure this works properly unary tests should be 
//  written).
//
#define WORD_CMPR_LOG8_BITS	4

//
// Number of bits encoding the compression model
//
#define WORD_CMPR_MODEL_BITS	2
//
// Compression model value for Decr
//
#define WORD_CMPR_MODEL_DECR	0
//
// Compression model value for Fixed
//
#define WORD_CMPR_MODEL_FIXED	1

class WordBitCompress : public WordBitStream {
 public:
  //-
  // Constructor. Create a empty stream.
  //
  WordBitCompress() { }
  //-
  // Constructor. Create a empty stream and pre-allocate <b>size</b>
  // bytes.
  //
  WordBitCompress(int size) : WordBitStream(size) { }

  //-
  // Put in bitstream integer value <b>v</b> (coded on <b>n</b> bits).
  //
  // The encoding is : 
  // <ul>
  // <li> WordBitStream::PutUint(log2(v), log2(n))
  // <li> WordBitStream::PutUint(v, log2(v))
  // </ul>
  //
  void         PutUint(unsigned int v, int n);
  //-
  // Get integer value from bitstream and return it (coded on <b>n</b> bits).
  //
  // The decoding is : 
  // <ul>
  // <li> nbits = WordBitStream::GetUint(log2(n))
  // <li> return WordBitStream::GetUint(nbits))
  // </ul>
  //
  unsigned int GetUint(int n);

  //-
  // Put in bitstream the array of <b>vals</b> integer values 
  // of size <b>n</b> and return the number of bits used in the bitstream.
  //
  // The encoding is : 
  // <ul>
  // <li> PutUint(n, WORD_CMPR_NBITS_NVALS)
  // <ul> 
  // If Decr preforms better than Fixed
  // <ul>
  // <li> PutUint(WORD_CMPR_MODEL_DECR, WORD_COMPRESS_MODEL_BITS)
  // <li> PutUintsDecr(vals, n)
  // </ul>
  // Otherwise, if Fixed preforms better than Decr
  // <ul>
  // <li> PutUint(WORD_CMPR_MODEL_FIXED, WORD_COMPRESS_MODEL_BITS)
  // <li> PutUintsDecr(vals, n)
  // </ul>
  // 
  int PutUints(unsigned int *vals, int n);
  //-
  // Get list of integer values from bitstream and return them in
  // the <b>valsp</b> argument. The length of the <b>vals</b> array
  // is returned. If 0 is returned, the <b>valsp</b> is set to null.
  // The <b>valsp</b> argument is allocated with <i>new</i>, it is
  // the responsibility of the caller to free it.
  //
  // The decoding is : 
  // <ul>
  // <li> count = GetUint(WORD_CMPR_NBITS_NVALS)
  // <li> model = GetUint(WORD_COMPRESS_MODEL_BITS)
  // <li> GetUintsFixed(vals, count) (if model == WORD_COMPRESS_MODEL_FIXED)
  // <li> GetUintsDecr(vals, count) (if model == WORD_COMPRESS_MODEL_DECR)
  // <ul> 
  //
  int GetUints(unsigned int **valsp);
  //-
  // Alias for GetUints(unsigned int **valsp). The <b>valsp</b> argument
  // must point to an allocated array of <b>valsp_size</b> bytes.
  // 
  int GetUints(unsigned int **valsp, int *valsp_size);

  //-
  // Put in bitstream the array of <b>vals</b> unsigned char values 
  // of size <b>n</b> and return the number of bits used in the bitstream.
  //
  // The encoding is : 
  // <ul>
  // <li> PutUint(n, WORD_CMPR_NBITS_NVALS)
  // <li> PutUint(log2(max of all vals), WORD_CMPR_LOG8_BITS)
  // <li> foreach val in vals : PutUint(val, log2(max of all vals))
  // <ul> 
  //
  int PutUchars(unsigned char *vals, int n);    
  //-
  // Get list of unsigned char values from bitstream and return them in
  // the <b>valsp</b> argument. The length of the <b>vals</b> array
  // is returned. If 0 is returned, the <b>valsp</b> is set to null.
  // The <b>valsp</b> argument is allocated with <i>new</i>, it is
  // the responsibility of the caller to free it.
  //
  // The decoding is : 
  // <ul>
  // <li> count = GetUint(WORD_CMPR_NBITS_NVALS)
  // <li> log2(max of all vals) = GetUint(WORD_CMPR_LOG8_BITS)
  // <li> count times : GetUint(log2(max of all vals))
  // </ul>
  //
  int GetUchars(unsigned char **valsp);
  //-
  // Alias for GetUchars(unsigned int **valsp). The <b>valsp</b> argument
  // must point to an allocated array of <b>valsp_size</b> bytes.
  // 
  int GetUchars(unsigned char **valsp, int *valsp_size);

  //-
  // Put in bitstream the array of <b>vals</b> integer values 
  // of size <b>n</b>.
  //
  // The encoding is : 
  // <ul>
  // <li> PutUint(log2(max of all vals), WORD_CMPR_LOG32_BITS)
  // <li> foreach val in vals : PutUint(val, log2(max of all vals))
  // </ul>
  //
  void PutUintsFixed(unsigned int *vals, int n);
  //-
  // Get list of integer values from bitstream and return them in
  // the <b>vals</b> argument. The array pointed by the <b>vals</b>
  // argument must be large enough to hold <b>n</b> elements.
  //
  // The decoding is : 
  // <ul>
  // <li> bits = GetUint(WORD_CMPR_LOG32_BITS)
  // <li> count times : GetUint(bits)
  // </ul>
  //
  void GetUintsFixed(unsigned int *vals, int n);

  //-
  // Put in bitstream the array of <b>vals</b> integer values 
  // of size <b>n</b>.
  //
  // The encoding uses an algorithm inspired by Shannon-Fano. 
  // The <b>vals</b> array is divided in chunks of equal size
  // and each number in a chunck is coded by the chunk number 
  // and its offset within the chunk. See VLengthCoder implementation
  // in WordBitCompress.cc for more information.
  //
  void PutUintsDecr(unsigned int *vals, int n);
  //-
  // Get list of integer values from bitstream and return them in
  // the <b>vals</b> argument. The array pointed by the <b>vals</b>
  // argument must be large enough to hold <b>n</b> elements.
  //
  void GetUintsDecr(unsigned int *vals, int n);
};

#endif /* _WordBitCompress_h */

