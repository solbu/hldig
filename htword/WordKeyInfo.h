// WordKeyInfo.h
//
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
typedef unsigned int WordKeyNum;

//
// Describes the structure of the key, ie meta information
// for the key. This includes the layout of the packed version
// stored on disk.
//

class WordKeyField
{
 public:
    String name;				// Symbolic name of the field
    int type;				// WORD_ISA_<type> of the field
    int lowbits;			// Packed info (see word_builder.pl)
    int lastbits;			// Packed info (see word_builder.pl)
    int bytesize;			// Packed info (see word_builder.pl)
    int bytes_offset;			// Packed info (see word_builder.pl)
    int bits;				// Packed info (see word_builder.pl)
//      int index;				// Index of the object in the pool_<type> array
    int direction;			// Sorting direction
    int encoding_position;              
    int sort_position;
    int bits_offset;


	
    void nprint(char c,int n);
    void show();
    WordKeyField(WordKeyField *previous,char *nname,int nbits,int encoding_position, int sort_position );
    WordKeyField(){;}
};


class WordKeyInfo;


class WordKeyInfo 
{
    friend WordKeyField;
protected:
    //
    // Array describing the fields, in encoding order. 
    //
#define WORD_SORT_ASCENDING	1
#define WORD_SORT_DESCENDING	2

public:
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

    static void Initialize( const Configuration &config);
    void        Initialize( int nnfields);
    void        Initialize( String &line);
    void        AddFieldInEncodingOrder(String &name,int bits, int sort_position);
    void        AddFieldInEncodingOrder(const String &line);
    void        SetDescriptionFromFile  (const String &filename);
    static void SetKeyDescriptionFromFile  (const String &filename);
    void        SetDescriptionFromString(const String &desc);
    static void SetKeyDescriptionFromString(const String &desc);

    void  show();

    static inline WordKeyInfo *Get(){return WordContext::key_info;}

    ~WordKeyInfo()
    {
	if(sort){delete [] sort;}
	if(sort){delete [] encode;}
    }
    WordKeyInfo()
    {
	sort   = NULL;
	nfields = -1;
    }

// DEBUGINIG / BENCHMARKING
    static void SetKeyDescriptionRandom(int maxbitsize=100,int maxnnfields=10);
};

#endif
