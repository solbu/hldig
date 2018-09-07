//
// WordDBCompress.h
//
// WordDBCompress: Implements specific compression scheme for
//                 Berkeley DB pages containing WordReferences objects.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordDBCompress.h,v 1.6 2004/05/28 13:15:26 lha Exp $
//

#ifndef _WordDBCompress_h_
#define _WordDBCompress_h_

// ***********************************************
// *************** WordDBCompress*****************
// ***********************************************
// Starting point for compression. 
// 
//
// Comrpession HOW IT WORKS:
//
// ** General outline: 
//
// BerkeleyDB pages are stored in a memory pool. When the memory pool
// is full, least recently used pages are swaped to disk.  Page
// compression occurs at page in/out level.  The
// WordDBCompress_compress_c functions are C callbacks that are called
// by the the page compression code in BerkeleyDB. The C callbacks the
// call the WordDBCompress comress/uncompress methods. The
// WordDBCompress creates a WordDBPage which does the actual
// compress/uncompress job.
// 
// The  WordDBPage compression/uncompression methods  store/retreive data
// from a bitstream.  BitStream is  a simple bitstream, and Compressor is
// a bitstream with added compression capabilities.
// 

// Compression algorithm.
// 
// Most DB pages are full of really redundant data. Mifluz choice of using 
// one db entry per word makes the DB pages have an even more redundant.
// But this choice also makes the pages have a very simple structure.
// 
// Here is a real world example of what a page can look like:
// (key structure: word + 4 numerical fields)
// 
// "trois"     1 4482    1  10b    
// "trois"     1 4482    1  142    
// "trois"     1 4484    1   40    
// "trois"     1 449f    1  11e    
// "trois"     1 4545    1   11    
// "trois"     1 45d3    1  545    
// "trois"     1 45e0    1  7e5    
// "trois"     1 45e2    1  830    
// "trois"     1 45e8    1  545    
// "trois"     1 45fe    1   ec    
// "trois"     1 4616    1  395    
// "trois"     1 461a    1  1eb    
// "trois"     1 4631    1   49    
// "trois"     1 4634    1   48    
// .... etc ....
// 
// To compress we chose to only code differences between succesive entries.
// 
// Differences in words are coded by 2 numbers and some letters: 
// - the position within the word of the first letter that changes
// - the size of the new suffix
// - the letters in the new suffix
// 
// Only differences in succesive numerical entries are stored.
// 
// A flag is stored for each entry indicating which fields have changed.
// 
// All this gives us a few numerical arrays which are themselves compressed 
// and sent to the bitstream.
//
//
class WordDBCompress
{
public:
  WordDBCompress ();
  WordDBCompress (int, int);

  int Compress (const u_int8_t * inbuff, int inbuff_length,
                u_int8_t ** outbuffp, int *outbuff_lengthp);
  int Uncompress (const u_int8_t * inbuff, int inbuff_length,
                  u_int8_t * outbuff, int outbuff_length);

  //
  // Return a new DB_CMPR_INFO initialized with characteristics of the
  // current object and suitable as WordDB::CmprInfo argument.
  //
  DB_CMPR_INFO *CmprInfo ();

private:
    DB_CMPR_INFO * cmprInfo;

  //ZLIB WordDBCompression Flags
  int use_zlib;
  int zlib_level;

// DEBUGING / BENCHMARKING
  int debug;
// 0 : no debug no check
// 1 : TestCompress before each compression (but no debug within Compress Uncompress)
// 2 : use_tags (BitStream) within TestCompress ->  Compress Uncompress
// 3 : verbose
  int TestCompress (const u_int8_t * pagebuff, int pagebuffsize);
};

#endif
