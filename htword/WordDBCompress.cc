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
// $Id: WordDBCompress.cc,v 1.1.2.15 2000/01/11 18:48:47 bosc Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <ctype.h>


#include "WordDBPage.h"
#include "WordDBCompress.h"
#include "WordBitCompress.h"

#include "HtTime.h"
#include "HtMaxMin.h"
#include "WordMonitor.h"



//byte a;

// ***********************************************
// *********** WordDBCompress  *******************
// ***********************************************

//word_key_info->sort[position].encoding_position

WordDBCompress::WordDBCompress()
{
    debug=1;
    bm_cmpr_count=0;
    bm_cmpr_time=0;
    bm_ucmpr_count=0;
    bm_ucmpr_time=0;
    bm_mxtreelevel=0;
    bm_nonleave_count=0;
    bm_cmpr_ratio=0;
    bm_cmpr_overflow=0;
}

extern "C"
{
/*
 *   WordDBCompress: C-callbacks, actually called by Berkeley-DB
 *      they just call their WordDBCompress equivalents (by using user_data)
 */
int WordDBCompress_compress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp, void *user_data)
{
/*    fprintf(stderr,"::WordDBCompress_compress_c:\n"); */
    if(!user_data){errr("word_db_page_compress::no user_data -> no WordDBCompress object");}
    return( ((WordDBCompress *)user_data)->Compress(inbuff,inbuff_length,outbuffp,outbuff_lengthp) );
}
int WordDBCompress_uncompress_c(const u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length, void *user_data)
{
/*    fprintf(stderr,"::WordDBCompress_uncompress_c:\n");*/
    if(!user_data){errr("word_db_page_uncompress::no user_data -> no WordDBCompress object");}
    return( ((WordDBCompress *)user_data)->Uncompress(inbuff,inbuff_length,outbuff,outbuff_length) );
}
}

//  #include"bmt_Profiling.h"
//  static bmt_Profiling bmt_Profiler(__FILE__);

int word_debug_cmprcount=0;
//  Compresses inbuff to outbuff
int 
WordDBCompress::Compress(const  u_int8_t *inbuff, int inbuff_length, u_int8_t **outbuffp, int *outbuff_lengthp)
{
//bmt_START;
    double start_time=HtTime::DTime();
    // create a page from inbuff
    WordDBPage pg(inbuff,inbuff_length);
//bmt_END;
//bmt_START;

    if(debug>2)
    {
	printf("###########################  WordDBCompress::Compress:%5d  #################################################\n",word_debug_cmprcount);
	pg.show();
	printf("~~~~~~~~~~~~~\n");
    }


//      pg.show();
    // DEBUG: check if decompressed compresed page is equivalent to original
    if(debug)TestCompress(inbuff,inbuff_length,debug);

//bmt_END;
//bmt_START;
    // do the real compression
    Compressor *res=pg.Compress(0, cmprInfo);
//bmt_END;

//bmt_START;
    // copy it to outbuff
    (*outbuffp)=res->get_data();
    (*outbuff_lengthp)=res->buffsize();
//bmt_END;


    if(debug>2)
    {
	res->show();
	printf("\n%%%%%%%% Final COMPRESSED size:%4d   %f\n",res->size(),res->size()/8.0);
	printf("***************************  %5d  #################################################\n",word_debug_cmprcount++);
    }

    delete res;
    if(debug>2){printf("WordDBCompress::Compress: final output size:%6d (inputsize:%6d)\n",(*outbuff_lengthp),inbuff_length);}

    // DEBUGING / BENCHMARKING
    {
	bm_cmpr_ratio+=(*outbuff_lengthp)/(double)inbuff_length;
	if( (*outbuff_lengthp) > inbuff_length/(1<<(cmprInfo->coefficient)) )
	{bm_cmpr_overflow++;}
	bm_cmpr_count++;
	if(pg.type!=5){bm_nonleave_count++;}
	bm_mxtreelevel=HtMAX(pg.pg->level,bm_mxtreelevel);
	bm_cmpr_time+=HtTime::DTime(start_time);
	(*monitor)();
    }

    // cleanup
    pg.unset_page();

    return(0);
}

//  Uncompresses inbuff to outbuff
int 
WordDBCompress::Uncompress(const u_int8_t *inbuff, int inbuff_length, u_int8_t *outbuff,int outbuff_length)
{
    double start_time=HtTime::DTime();

    if(debug>2){printf("WordDBCompress::Uncompress::  %5d -> %5d\n",inbuff_length,outbuff_length);}
    // create a page for decompressing into it
    WordDBPage pg(outbuff_length);
    if(debug>2){printf("------------------------  WordDBCompress::Uncompress:%5d --------------------------------\n",word_debug_cmprcount);}

    // create a Compressor from inbuff and setit up
    Compressor in(inbuff_length);
    in.set_data(inbuff,inbuff_length*8);
    in.rewind();

    // do the uncompression
    pg.Uncompress(&in,0);
    
    // copy the result to outbuff
    memcpy((void *)outbuff,(void *)pg.pg,outbuff_length);

    if(debug>2){printf("------------------------  WordDBCompress::Uncompress: END %d\n",word_debug_cmprcount);}


    // DEBUGING / BENCHMARKING
    {
	bm_ucmpr_count++;
	if(pg.type!=5){bm_nonleave_count++;}
	bm_mxtreelevel=HtMAX(pg.pg->level, bm_mxtreelevel);
	bm_ucmpr_time+=HtTime::DTime(start_time);
	(*monitor)();
    }

    pg.delete_page();
    return(0);
}

// checks if compression/decompression sequence is harmless
int
WordDBCompress::TestCompress(const  u_int8_t* pagebuff, int pagebuffsize,int debuglevel)
{
    WordDBPage pg(pagebuff,pagebuffsize);
    pg.TestCompress(debuglevel);
    pg.unset_page();
    return 0;
}

