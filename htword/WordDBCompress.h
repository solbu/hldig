
#ifndef _WordDBCompress_h_
#define _WordDBCompress_h_

#include "WordList.h"
extern "C"
{
#include "zlib.h"
#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "db_page.h"
#include "common_ext.h"
}
class WordDBCompress
{
 public:
    int debug;
// 0 : no debug no check
// 1 : TestCompress before each compression (but no debug within Compress Uncompress)
// 2 : use_tags (BitStream) within TestCompress ->  Compress Uncompress
// 3 : verbose
    int TestCompress(const  u_int8_t* pagebuff, int pagebuffsize,int debuglevel);
    int Compress(const  u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp);
    int Uncompress(const  u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length);
    WordDBCompress();
};


extern WordDBCompress wordDBCompress;

extern "C"
{
    extern int WordDBCompress_compress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp, void *user_data);
    extern int WordDBCompress_uncompress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length, void *user_data);
}
#endif
