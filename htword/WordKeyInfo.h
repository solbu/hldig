// WordKeyInfo.h
//
// NAME
// information on the key structure of the inverted index.
//
// SYNOPSIS
//
// Helper for the WordKey class.
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
//   In the following explanation of the <i><desc></i> format,
//   mandatory words are
//   in bold and values that must be replaced in italic.
//   <br>
//   <b>Word</b> <i>bits/name bits</i>[/...]
//   <br>
//   The <i>name</i> is an alphanumerical symbolic name for the key field.
//   The <i>bits</i> is the number of bits required to store this field.
//   Note that all values are stored in unsigned integers (unsigned int).
//   Example:
//   <pre>
//   Word 8/Document 16/Location 8
//   </pre>
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
//

#ifndef _WordKeyInfo_h_
#define _WordKeyInfo_h_

#include "Configuration.h"

//
// Maximum number of fields in a key description
//
#define WORD_KEY_MAX_NFIELDS 7

#ifndef SWIG
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
      bits = 0;
    }

    inline WordKeyNum MaxValue() const {
      if(bits == 32) return 0xffffffff;
      else return ((1 << bits) - 1);
    }
    
    int bits;				// Size of field in bits
    String name;                        // Symbolic name of the field
};
#endif /* SWIG */

//
// Description of the key structure
//
class WordKeyInfo 
{
 public:
    WordKeyInfo(const Configuration& config);

#ifndef SWIG

    int         Set(const String &desc);

#endif /* SWIG */

    inline int NFields() { return nfields; }
#ifndef SWIG
    inline WordKeyNum MaxValue(int position) { return fields[position].MaxValue(); }

    //
    // Array describing the fields, in sort order.
    //
    WordKeyField fields[WORD_KEY_MAX_NFIELDS];
    //
    // Total number of fields
    //
    int nfields;
#endif /* SWIG */
};

#endif
