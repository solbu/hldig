//
// WordBitCompress.h
//
// BitStream: put and get bits into a buffer
//           *tagging:  add tags to keep track of the position of data 
//                      inside the bitstream for debuging purposes.
//           *freezing: saves current position. further inserts in the BitStream
//                      aren't really done. This way you can try different
//                      compression algorithms and chose the best.
//
// Compressor: BitStream with extended fuctionalities
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordBitCompress.h,v 1.1.2.4 1999/12/14 18:31:31 bosc Exp $
//

#ifndef   _WordBitCompress_h
#define  _WordBitCompress_h

#include<stdio.h>
#include<stdlib.h>
#include"HtVector_int.h"

typedef unsigned char byte;
// ******** HtVector_byte (header)
#define GType byte
#define HtVectorGType HtVector_byte
#include "HtVectorGeneric.h"

typedef char * charptr;
// ******** HtVector_charptr (header)
#define GType charptr
#define HtVectorGType HtVector_charptr
#include "HtVectorGeneric.h"


// ******** Utility inline functions and macros

// error checking
#define FATAL_ABORT fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);(*(int *)NULL)=1
#define errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);FATAL_ABORT;}
#define CHECK_MEM(p) if(!p) errr("mifluz: Out of memory!");
// max/min of 2 values
#define TMax(a,b) (((a)>(b)) ? (a) : (b))
#define TMin(a,b) (((a)<(b)) ? (a) : (b))

// compute integer log2
inline int
num_bits(unsigned int maxval )
{
    unsigned int mv=maxval;
    int nbits;
    for(nbits=0;mv;nbits++){mv>>=1;}
    return(nbits);
}
// compute 2^x
#define pow2(x) (1<<(x))


// function declarations
char *label_str(char *s,int n);
void  show_bits(int v,int n=16);
unsigned short max_v(unsigned short *vals,int n);
unsigned int   max_v(unsigned int   *vals,int n);
unsigned short min_v(unsigned short *vals,int n);
unsigned int   min_v(unsigned int   *vals,int n);





// **************************************************
// *************** BitStream  ***********************
// **************************************************
//  compression is done in Compressor not in BitStream
class BitStream
{
protected:

    // the buffer were the bitstream is stored
    HtVector_byte buff;

    // current bit position within the buffer
    int bitpos;

    // tags for debuging
    HtVector_int tagpos;
    HtVector_charptr tags;
    int use_tags;

    // freezing the bitstream
    HtVector_int freeze_stack;
    int freezeon;
public:
    void freeze();
    int unfreeze();

    // puts a bit into the bitstream : ** all puts go through this **
    void put(unsigned int v,char *tag=NULL)
    {
	if(freezeon){bitpos++;return;}
	if(v){buff.back()|=pow2(bitpos%8);}
	add_tag(tag);
	bitpos++;
	if(!(bitpos%8))// new byte
	{
	    buff.push_back(0);
	}
    }	

    // gets a bit from the bitstream : ** all gets go through this **
    byte get(char *tag=NULL)
    {
	if(check_tag(tag)==NOTOK){errr("BitStream::get() check_tag failed");}
	if(bitpos>=8*buff.size()){errr("BitStream::get reading past end of BitStream!");}
	byte res=buff[bitpos/8] & pow2(bitpos%8);
	bitpos++;
	return(res);
    }

    // get/put an integer using n bits
    void         put(unsigned int v,int n,char *tag="NOTAG");
    unsigned int get(               int n,char *tag=NULL);

    // get/put n bits of data stored in vals
    void put_zone(byte *vals,int n,char *tag);
    void get_zone(byte *vals,int n,char *tag);

    // 
    void add_tag(char *tag);
    int  check_tag(char *tag,int pos=-1);
    void set_use_tags(){use_tags=1;}
    int  find_tag(char *tag);
    int  find_tag(int pos,int posaftertag=1);

    void show_bits(int a,int n);
    void show(int a=0,int n=-1);

    // position accesors
    int size(){return(bitpos);}
    int buffsize(){return(buff.size());}

    // get a copy of the buffer
    byte *get_data();
    // set the buffer from outside data (current buffer must be empty)
    void set_data(const byte *nbuff,int nbits);
      
    // use this for reading a BitStream after you have written in it 
    // (generally for debuging)
    void rewind(){bitpos=0;}

    ~BitStream()
    {
	int i;
	for(i=0;i<tags.size();i++){free(tags[i]);}
    }
    BitStream(int size)
    {
	init();
    }
    BitStream()
    {
	init();
    }
 private:
    void init()
    {
	bitpos=0;
	buff.push_back(0);
	freezeon=0;
	use_tags=0;
    }
};


// **************************************************
// *************** Compressor ***********************
// **************************************************

// Constants used by Compressor
// number of bits to code the number of values in an array 
#define NBITS_NVALS 16
// number of bits to code the values in an unsigned int array (=sizeof(unsigned int))
#define NBITS_VAL 32
// number of bits to code he number of bits used by an unsigned int value
#define NBITS_NBITS_VAL  5
// number of bits to code the number of bits used by a byte value
#define NBITS_NBITS_CHARVAL 4

class Compressor : public BitStream
{
public:
    int verbose;
    // compress/decompress an array of unsigned ints (choosing best method)
    int put_vals(unsigned int *vals,int n,char *tag);
    int get_vals(unsigned int **pres,char *tag="BADTAG!");

    // compress/decompress an array of bytes (very simple)
    int put_fixedbitl(byte *vals,int n,char *tag);    
    int get_fixedbitl(byte **pres,char *tag="BADTAG!");

    // compress/decompress an array of unsigned ints (very simple)
    void get_fixedbitl(unsigned int *res,int n);
    void put_fixedbitl(unsigned int *vals,int n);

    // compress/decompress an array of unsigned ints (sophisticated)
    void get_decr(unsigned int *res,int n);
    void put_decr(unsigned int *vals,int n);

    Compressor():BitStream()
	{
	    verbose=0;
	}
    Compressor(int size):BitStream(size)
	{
	    verbose=0;
	}

};



#endif
