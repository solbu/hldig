//
// WordDBCompress.h
//
// WordDBCompress: Implements specific compression scheme for
//                 Berkeley DB pages containing WordReferences objects.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDBCompress.h,v 1.1.2.4 1999/12/21 17:31:48 bosc Exp $
//

#ifndef _WordDBCompress_h_
#define _WordDBCompress_h_

class WordDBCompress
{
 public:
    int Compress(const  u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp);
    int Uncompress(const  u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length);
    WordDBCompress();

// DEBUGING / BENCHMARKING
    int debug;
// 0 : no debug no check
// 1 : TestCompress before each compression (but no debug within Compress Uncompress)
// 2 : use_tags (BitStream) within TestCompress ->  Compress Uncompress
// 3 : verbose
    int TestCompress(const  u_int8_t* pagebuff, int pagebuffsize,int debuglevel);
    int    cmpr_count;
    double total_cmpr_time;
    int    ucmpr_count;
    double total_ucmpr_time;
    int    mxtreelevel;
};


extern WordDBCompress wordDBCompress;

extern "C"
{
    extern int WordDBCompress_compress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp, void *user_data);
    extern int WordDBCompress_uncompress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length, void *user_data);
}
#endif
