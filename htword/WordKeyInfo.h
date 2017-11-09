// WordKeyInfo.h
//
// NAME
// information on the key structure of the inverted index.
//
// SYNOPSIS
//
// Use the WordKey::NField() method instead.
//
// DESCRIPTION
//
// Describe the structure of the index key (<i>WordKey</i>).
// The description includes the layout of the packed version
// stored on disk.
//
// CONFIGURATION
//
// wordlist_wordkey_description <desc> (no default)
//   Describe the structure of the inverted index key.
//   In the following explanation of the <i><desc></i> format
//   mandatory words are
//   in bold and values that must be replaced in italic.
//   <br>
//   <b>Word</b>/<i>name bits</i>[/...]
//   <br>
//   The <i>name</i> is an alphanumerical symbolic name for the key field.
//   The <i>bits</i> is the number of bits required to store this field.
//   Note that all values are stored in unsigned integers (unsigned int).
//
//
// END
//   
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//

#ifndef _WordKeyInfo_h_
#define _WordKeyInfo_h_

#include "Configuration.h"

//
// Type number associated to each possible type for a key element
// (type field of struct WordKeyInfo).
//
#define WORD_ISA_NUMBER		1
#define WORD_ISA_STRING		2

//
// Maximum number of fields in a key description
//
#define WORD_KEY_MAX_NFIELDS 20

//
// All numerical fields of the key are typed WordKeyNum.
// Most of the code strongly assume that it is unsigned. 
// Mainly provided to be replaced by unsigned longlong WordKeyNum
// for 64 bits machines.
//
typedef unsigned int WordKeyNum;

//
// Maximum number of bits in a field
//
#define WORD_KEY_MAXBITS	((int)(sizeof(WordKeyNum) * 8))
#define WORD_KEY_MAXVALUE	((WordKeyNum)~(WordKeyNum)0)

//
// Description of a single field
//
class WordKeyField
{
 public:
    WordKeyField() {
      type = lowbits = lastbits = bytesize = bytes_offset = bits = bits_offset = 0;
    }

    //
    // Precompute information that will be needed to pack/unpack the key
    // to/from disk.
    // 
    // The <previous> field is used to compute the position of the field
    // in packed string.  <nname> is the symbolic name of the field
    // <nbits> is the number of bits actualy used in a number.
    //
    int SetNum(WordKeyField *previous, char *nname, int nbits);
    //
    // Set the one and only string field
    //
    int SetString();

    //
    // Maximum possible value for this field.
    //
    WordKeyNum MaxValue() const {
      return bits >= WORD_KEY_MAXBITS ? WORD_KEY_MAXVALUE : ((1 << bits) - 1);
    }

    //
    // Debugging and printing
    //
    void Show();

    String name;			// Symbolic name of the field
    int type;				// WORD_ISA_{STRING|NUMBER} 
    //
    // 01234567012345670123456701234567
    // +-------+-------+-------+-------+--
    //    100101010011100111101011110
    // ^^^                     ^^^^^^
    //   |                        |
    // lowbits = 3           lastbits = 6
    //
    int lowbits;			
    int lastbits;			
    int bytesize;			// Number of bytes involved
    int bytes_offset;			// Offset of first byte from start
    int bits;				// Size of field in bits
    int bits_offset;                    // Offset of first bit from start
};

//
// Description of the key structure
//
class WordKeyInfo 
{
 public:
    WordKeyInfo(const Configuration& config);
    ~WordKeyInfo() { if(sort) delete [] sort; }

    //
    // Unique instance handlers 
    //
    static void Initialize(const Configuration& config);
    static void InitializeFromString(const String &desc);
    static WordKeyInfo* Instance() {
      if(instance) return instance;
      fprintf(stderr, "WordKeyInfo::Instance: no instance\n");
      return 0;
    }

    int         Alloc(int nnfields);
    int         Set(const String &desc);

    void  Show();

    //
    // Array describing the fields, in sort order.
    //
    WordKeyField *sort;
    //
    // Total number of fields
    //
    int nfields;
    //
    // Total number of bytes used by numerical fields
    //
    int num_length;

    //
    // Unique instance pointer
    //
    static WordKeyInfo* instance;
};

#endif
