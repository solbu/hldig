//
// WordDBCompress.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDBCompress.h,v 1.1.2.9 2000/09/14 03:13:27 ghutchis Exp $
//

#ifndef _WordDBCompress_h_
#define _WordDBCompress_h_

#include "WordDBCompress.h"

#include "db.h"

class WordMonitor;
class WordDBEncoded;
class WordContext;

class WordDBCompress
{
 public:
    WordDBCompress(WordContext* ncontext);
    ~WordDBCompress();

    //
    // Entry points
    //
    int Compress(const  unsigned char* inbuff, int inbuff_length, unsigned char** outbuffp, int* outbuff_lengthp);
    int Uncompress(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int outbuff_length);

    int CompressBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int* outbuff_lengthp);
    int CompressIBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int* outbuff_lengthp);
    int CompressLBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int* outbuff_lengthp);

    int UncompressBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int outbuff_length);
    int UncompressIBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int outbuff_length);
    int UncompressLBtree(const  unsigned char* inbuff, int inbuff_length, unsigned char* outbuff, int outbuff_length);
    
    //
    // Return a new DB_CMPR_INFO initialized with characteristics of the
    // current object and suitable as WordDB::CmprInfo argument.
    //
    DB_CMPR_INFO *CmprInfo();

    //
    // Debugging
    //
    void DumpPage(const unsigned char* page) const;
    int DiffPage(const unsigned char* first, const unsigned char* second) const;

 private:
    DB_CMPR_INFO *cmprInfo;
    WordContext *context;
    WordDBEncoded *encoded;

    int verbose;
    int debug;
};

#endif /* _WordDB_h */
