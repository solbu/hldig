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
// $Id: WordDBCompress.cc,v 1.7 2004/05/28 13:15:26 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <ctype.h>

#include "WordDBPage.h"
#include "WordDBCompress.h"
#include "WordBitCompress.h"

/*
 *   WordDBCompress: C-callbacks, actually called by Berkeley-DB
 *      they just call their WordDBCompress equivalents (by using user_data)
 */
extern "C"
{

static int WordDBCompress_compress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp, void *user_data)
{
    if(!user_data) {
      fprintf(stderr, "WordDBCompress_compress_c:: user_data is NULL");
      return NOTOK;
    }
    return ((WordDBCompress *)user_data)->Compress((unsigned char*)inbuff, inbuff_length, (unsigned char**)outbuffp, outbuff_lengthp);
}

static int WordDBCompress_uncompress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length, void *user_data)
{
    if(!user_data) {
      fprintf(stderr, "WordDBCompress_uncompress_c:: user_data is NULL");
      return NOTOK;
    }
    return ((WordDBCompress *)user_data)->Uncompress((unsigned char *)inbuff, inbuff_length, (unsigned char*)outbuff, outbuff_length);
}

}

// ***********************************************
// *********** WordDBCompress  *******************
// ***********************************************

WordDBCompress::WordDBCompress()
{
  
  cmprInfo = 0;

  //
  // DEBUGING / BENCHMARKING
  //
  debug = 0;

  //zlib WordDB Compression
  use_zlib = 0;
  zlib_level = 0;

}


WordDBCompress::WordDBCompress(int zlib, int level)
{

  cmprInfo = 0;

  //
  // DEBUGING / BENCHMARKING
  //
  debug = 0;

  //zlib WordDB Compression
  use_zlib = zlib;
  zlib_level = level;
}


DB_CMPR_INFO* WordDBCompress::CmprInfo()
{

  DB_CMPR_INFO *cmpr_info = new DB_CMPR_INFO;

  cmpr_info->user_data = (void *)this;
  cmpr_info->compress = WordDBCompress_compress_c;
  cmpr_info->uncompress = WordDBCompress_uncompress_c;
  cmpr_info->coefficient = 3;  // reduce page size by factor of 1<<3 = 8
  cmpr_info->max_npages = 9;

  if(use_zlib == 1)
      cmpr_info->zlib_flags = zlib_level;
  else
      cmpr_info->zlib_flags = 0;
  
  cmprInfo = cmpr_info;
  
  return cmpr_info;
}

int 
WordDBCompress::Compress(const  u_int8_t *inbuff, int inbuff_length, u_int8_t **outbuffp, int *outbuff_lengthp)
{
  WordDBPage pg(inbuff, inbuff_length);

  if(debug > 2) {
    printf("###########################  WordDBCompress::Compress:  #################################################\n");
    pg.show();
    printf("~~~~~~~~~~~~~\n");
  }

  if(debug) TestCompress(inbuff, inbuff_length);

  Compressor *res = pg.Compress(0, cmprInfo);

  (*outbuffp) = res->get_data();
  (*outbuff_lengthp) = res->buffsize();

  if(debug > 2) {
    res->show();
    printf("\n%%%%%%%% Final COMPRESSED size:%4d   %f\n",res->size(),res->size()/8.0);
    printf("***************************   #################################################\n");
  }

  delete res;
  if(debug > 2) printf("WordDBCompress::Compress: final output size:%6d (inputsize:%6d)\n", (*outbuff_lengthp), inbuff_length);

  pg.unset_page();

  return(0);
}

int 
WordDBCompress::Uncompress(const u_int8_t *inbuff, int inbuff_length, u_int8_t *outbuff,int outbuff_length)
{
  if(debug > 2) printf("WordDBCompress::Uncompress::  %5d -> %5d\n", inbuff_length, outbuff_length);

  WordDBPage pg(outbuff_length);

  if(debug > 2) printf("------------------------  WordDBCompress::Uncompress: --------------------------------\n");

  Compressor in(inbuff_length);
  in.set_data(inbuff,inbuff_length*8);
  in.rewind();

  pg.Uncompress(&in,0);
    
  memcpy((void *)outbuff, (void *)pg.pg, outbuff_length);

  if(debug > 2) printf("------------------------  WordDBCompress::Uncompress: END\n");

    // DEBUGING / BENCHMARKING

  pg.delete_page();
  return(0);
}

int
WordDBCompress::TestCompress(const  u_int8_t* pagebuff, int pagebuffsize)
{
    WordDBPage pg(pagebuff,pagebuffsize);
    pg.TestCompress(debug);
    pg.unset_page();
    return 0;
}
