#ifndef   _WordBitCompress_h
#define  _WordBitCompress_h

#define TMax(a,b) (((a)>(b)) ? (a) : (b))
#define TMin(a,b) (((a)<(b)) ? (a) : (b))
#include<stdio.h>
#include<stdlib.h>
#include"HtVector_int.h"
#define fatal fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);(*(int *)NULL)=1
#define errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);fatal;}
#define CHECK_MEM(p) if(!p) errr("mifluz: Out of memory!");

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

// compute log2
inline int
num_bits(unsigned int maxval )
{
    unsigned int mv=maxval;
    int nbits;
    for(nbits=0;mv;nbits++){mv>>=1;}
    return(nbits);
}
#define pow2(x) (1<<(x))
char *label_str(char *s,int n);




// **************************************************
// *************** BitStream  ***********************
// **************************************************
#define NBITS_NVALS 16
class BitStream
{
protected:
    HtVector_byte buff;
    int bitpos;
    HtVector_int tagpos;
    HtVector_charptr tags;
    HtVector_int freeze_stack;
    int freezeon;
    int use_tags;
public:
    void freeze()
    {
	freeze_stack.push_back(bitpos);
	freezeon=1;
    }
    int unfreeze()
    {
	int size=bitpos;
	bitpos=freeze_stack.back();
	freeze_stack.pop_back();
	size-=bitpos;
	if(freeze_stack.size()==0){freezeon=0;}
	return(size);
    }

    void add_tag(char *tag)
    {
	if(!use_tags){return;}
	if(freezeon){return;}
	if(!tag){return;}
	tags.push_back(strdup(tag));
	tagpos.push_back(bitpos);
    }
    void put_zone(byte *vals,int n,char *tag)
    {
	add_tag(tag);
	for(int i=0;i<(n+7)/8;i++){put(vals[i],TMin(8,n-8*i),NULL);}
    }
    void get_zone(byte *vals,int n,char *tag)
    {
	check_tag(tag);
	for(int i=0;i<(n+7)/8;i++){vals[i]=get(TMin(8,n-8*i));}
    }
    void put(unsigned int v,char *tag=NULL)
    {
	if(freezeon){bitpos++;return;}
//    	printf("%c",(v ? '1' : '0'));
	if(v){buff.back()|=pow2(bitpos%8);}
	add_tag(tag);
	bitpos++;
	if(!(bitpos%8))// new byte
	{
	    buff.push_back(0);
	}
    }	
    void put(unsigned int v,int n,char *tag="NOTAG")
    {
	int i;
	add_tag(tag);
//  	if(n==15){printf(":%3d: ",v);}
	for(i=0;i<n;i++)
	{
	    put((v& pow2(i) ? 1:0));
//  	    printf("%c",v& pow2(i) ? '1' : '0');
	}
//  	    printf(" ");
    }



    byte get(char *tag=NULL)
    {
	if(check_tag(tag)==NOTOK){fatal;}
	if(bitpos>=8*buff.size()){errr("BitStream::get reading past end of BitStream!");}
	byte res=buff[bitpos/8] & pow2(bitpos%8);
	bitpos++;
//    	printf("%02x:%c ",buff[bitpos/8],(res ? '1' : '0'));
//    	printf("%c",(res ? '1' : '0'));
	return(res);
    }
    unsigned int get(int n,char *tag=NULL)
    {	
	if(check_tag(tag)==NOTOK){fatal;}
	unsigned int res=0;
	for(int i=0;i<n;i++)
	{
	    if(get()){res|=pow2(i);}
	}
//  	if(n==15){printf("%3d ",res);}
	return(res);
    }

    int  check_tag(char *tag,int pos=-1);
    int find_tag(char *tag);
    int find_tag(int pos,int posaftertag=1);
    void show_bits(int a,int n);
    void show(int a=0,int n=-1);

    int size(){return(bitpos);}
    int buffsize(){return(buff.size());}
    byte *get_data()
    {
	byte *res=(byte *)malloc(buff.size());
	CHECK_MEM(res);
	for(int i=0;i<buff.size();i++){res[i]=buff[i];}
	return(res);
    }
    void set_data(const byte *nbuff,int nbits)
    {
	if(buff.size()!=1 || bitpos!=0)
	{
	    printf("BitStream:set_data: size:%d bitpos:%d\n",buff.size(),bitpos);
	    errr("BitStream::set_data: valid only if BitStream is empty");
	}
	buff[0] = nbuff[0];
	for(int i=1;i<(nbits+7)/8;i++){buff.push_back(nbuff[i]);}
	bitpos=nbits;
    }
    void set_use_tags(){use_tags=1;}
      
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

extern int TMPDISPLAY;
extern unsigned int max_v(unsigned int *vals,int n);

// **************************************************
// *************** Compressor ***********************
// **************************************************



class Compressor : public BitStream
{
public:
    int put_vals(unsigned int *vals,int n,char *tag);
    int get_vals(unsigned int **pres,char *tag="BADTAG!");
    int put_fixedbitl(byte *vals,int n,char *tag);

    
    void get_fixedbitl(unsigned int *res,int n);
    void get_decr(unsigned int *res,int n);
    int get_fixedbitl(byte **pres,char *tag="BADTAG!");


    void compress_decr(unsigned int *vals,int n);
    void compress_fixed(unsigned int *vals,int n);
    Compressor():BitStream()
	{
	    ;
	}
    Compressor(int size):BitStream(size)
	{
	    ;
	}

};


void show_bits(int v,int n=16);
unsigned short max_v(unsigned short *vals,int n);
unsigned int max_v(unsigned int *vals,int n);
unsigned short min_v(unsigned short *vals,int n);
unsigned int min_v(unsigned int *vals,int n);


#endif
