// WordKeyInfo.h
//
// WordKeyInfo: Describe the structure of the index key (WordKey)
//              The description includes the layout of the packed version
//              stored on disk.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//

#ifndef _WordKeyInfo_h_
#define _WordKeyInfo_h_

#include "WordContext.h"
#include "Configuration.h"

//
// Type number associated to each possible type for a key element
// (type field of struct WordKeyInfo).
//
#define WORD_ISA_NUMBER		1
#define WORD_ISA_STRING		2

//
// All numerical fields of the key are typed WordKeyNum
//
typedef unsigned int WordKeyNum;

class WordKeyField
{
 public:
    WordKeyField(WordKeyField *previous, char *nname, int nbits, int encoding_position, int sort_position);
    WordKeyField() { }

    void Nprint(char c,int n);
    void Show();

    String name;			// Symbolic name of the field
    int type;				// WORD_ISA_{STRING|NUMBER} of the field
    int lowbits;			// 
    int lastbits;			
    int bytesize;			
    int bytes_offset;			
    int bits;				
    int direction;			// Sorting direction
    int encoding_position;              
    int sort_position;
    int bits_offset;
};

#define WORD_SORT_ASCENDING	1
#define WORD_SORT_DESCENDING	2

class WordKeyInfo 
{
 public:
    WordKeyInfo()
    {
	sort = NULL;
	nfields = -1;
    }
    ~WordKeyInfo()
    {
	if(sort) { delete [] sort; }
	if(encode) { delete [] encode; }
    }
    
    static void Initialize(const Configuration &config);
    void        Initialize(int nnfields);
    void        Initialize(String &line);
    void        AddFieldInEncodingOrder(String &name, int bits, int sort_position);
    void        AddFieldInEncodingOrder(const String &line);
    void        SetDescriptionFromFile(const String &filename);
    static void SetKeyDescriptionFromFile(const String &filename);
    void        SetDescriptionFromString(const String &desc);
    static void SetKeyDescriptionFromString(const String &desc);

    void  Show();

    static inline WordKeyInfo *Get(){return WordContext::key_info;}

    //
    // Build a random description key for test purpose.
    //
    static void SetKeyDescriptionRandom(int maxbitsize=100, int maxnnfields=10);

    //
    // Array describing the fields, in sort order.
    //
    WordKeyField *sort;
    //
    // Array describing the fields, in encoding order.
    //
    WordKeyField *encode;
    //
    // Total number of fields
    //
    int nfields;
    //
    // Minimum length of key on disk
    //
    int minimum_length;

    WordKeyField *previous;
    int encoding_position;
};

#endif
