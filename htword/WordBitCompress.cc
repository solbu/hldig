
#include"WordBitCompress.h"

// ******** HtVector_byte (implementation)
#define GType byte
#define HtVectorGType HtVector_byte
#include "HtVectorGenericCode.h"

// ******** HtVector_charptr (implementation)
#define GType charptr
#define HtVectorGType HtVector_charptr
#include "HtVectorGenericCode.h"


#include <stdlib.h>

int TMPDISPLAY=0;





// **************************************************
// *************** misc functions *******************
// **************************************************

char *
label_str(char *s,int n)
{
    static char buff[1000];
    sprintf(buff,"%s%d",s,n);
    return buff;
}

void
show_bits(int v,int n/*=16*/)
{
    int i;
    for(i=0;i<n;i++)
    {
	printf("%c",( v&(1<<(n-i-1)) ? '1':'0' ) );
    }
}

unsigned int
max_v(unsigned int *vals,int n)
{
    unsigned int maxv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned int v=vals[i];
	if(v>maxv){maxv=v;}
    }
    return(maxv);
}
unsigned short
max_v(unsigned short *vals,int n)
{
    unsigned short maxv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned short v=vals[i];
	if(v>maxv){maxv=v;}
    }
    return(maxv);
}
unsigned int
min_v(unsigned int *vals,int n)
{
    unsigned int minv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned int v=vals[i];
	if(v<minv){minv=v;}
    }
    return(minv);
}
unsigned short
min_v(unsigned short *vals,int n)
{
    unsigned short minv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned short v=vals[i];
	if(v<minv){minv=v;}
    }
    return(minv);
}
unsigned int *
duplicate(unsigned int *v,int n)
{
    unsigned int *res=new (unsigned int)[n];
    CHECK_MEM(res);
    memcpy((void *)res,(void *)v,n*sizeof(unsigned int));
    return(res);
}
int
qsort_uint_cmp(const void *a,const void *b)
{
    return 
	(*((unsigned int *)a)) -
	(*((unsigned int *)b))   ;
}
void
qsort_uint(unsigned int *v,int n)
{
    qsort((void *)v,(unsigned int)n,sizeof(unsigned int),&qsort_uint_cmp);
}

int
log2(unsigned int v)
{
    int res;
    for(res=-1;v;res++){v>>=1;}
    return(res);
}




// **************************************************
// *************** VlengthCoder   *******************
// **************************************************

class VlengthCoder
{
    int nbits;// min number of bits to code all entries
    int nlev;// split proba into 2^nlev parts
    int nintervals;

    int *intervals;
    int bitpos;
    BitStream &bs;
public:
    int code(unsigned int v)
    {
	int i;
	unsigned int lboundary=0;
	unsigned int sboundary=0;
	for(i=0;;i++)
	{
	    sboundary=lboundary+intervalsize(i);
	    if( (lboundary!=sboundary && v>=lboundary && v<sboundary) || 
		(lboundary==sboundary && v==lboundary)                   ){break;}
	    lboundary=sboundary;
	}
	// were in the i'th interval;
  	bs.put(i,nlev,"int");// store interval
	int bitsremaining=(intervals[i]>0 ? intervals[i]-1 : 0);
	if(TMPDISPLAY)printf("v:%6d interval:%2d (%5d - %5d) bitsremaining:%2d ",v,i,lboundary,sboundary,bitsremaining);
	v-=lboundary;
	if(TMPDISPLAY)printf("remain:%6d  totalbits:%2d\n",v,bitsremaining+nlev);
    	bs.put(v,bitsremaining,"rem");
	return(bitsremaining + nlev);
    }
    void code_begin()
    {
	int i;
	bs.add_tag("VlengthCoder:Header");
	bs.put(nbits,5,"nbits");
	bs.put(nlev,5,"nlev");
	for(i=0;i<nintervals;i++)
	{
	    bs.put(intervals[i],5,label_str("interval",i));
	}
    }
    void get_begin()
    {
	int i;
	nbits=bs.get(5,"nbits");
	if(TMPDISPLAY)printf("get_begin nbits:%d\n",nbits);
	nlev=bs.get(5,"nlev");
	if(TMPDISPLAY)printf("get_begin nlev:%d\n",nlev);
	nintervals=pow2(nlev);

	intervals=new int [nintervals];
	CHECK_MEM(intervals);

	for(i=0;i<nintervals;i++)
        {
	    intervals[i]=bs.get(5,label_str("interval",i));
	    if(TMPDISPLAY)printf("get_begin intervals:%2d:%2d\n",i,intervals[i]);
	}
    }
    unsigned int get()
    {
	int i=bs.get(nlev,"int");// get interval
	if(TMPDISPLAY)printf("get:interval:%2d ",i);
	int bitsremaining=(intervals[i]>0 ? intervals[i]-1 : 0);
	if(TMPDISPLAY)printf("bitsremain:%2d ",bitsremaining);
	unsigned int v=bs.get(bitsremaining,"rem");
	if(TMPDISPLAY)printf("v0:%3d ",v);
	unsigned int lboundary=0;
	for(int j=0;j<i;j++){lboundary+=intervalsize(j);}
	v+=lboundary;
	if(TMPDISPLAY)printf("lboundary:%5d v:%5d \n",lboundary,v);
	return(v);
    }
    VlengthCoder(BitStream &nbs):bs(nbs)
    {
	nbits=0;
	nlev=0;
	nintervals=0;
	intervals=NULL;
	bitpos=0;
    }
    
    ~VlengthCoder()
    {
	delete [] intervals;
    }
    unsigned int intervalsize(int i){return((intervals[i] > 0 ? pow2(intervals[i]-1) : 0));}
    VlengthCoder(unsigned int *vals,int n,BitStream &nbs):bs(nbs)
    {
	unsigned int *sorted=duplicate(vals,n);
	qsort_uint(sorted,n);

	bitpos=0;
	nbits=num_bits(max_v(vals,n));
	nlev=5;
	nintervals=pow2(nlev);
	int i;
	intervals=new int [nintervals];
	CHECK_MEM(intervals);

	// find split boundaires
	if(TMPDISPLAY)printf("nbits:%d nlev:%d nintervals:%d \n",nbits,nlev,nintervals);
	unsigned int lboundary=0;
	for(i=0;i<nintervals-1;i++)
	{
	    unsigned int boundary=sorted[(n*(i+1))/nintervals];
	    intervals[i]=1+log2(boundary-lboundary);
	    if(TMPDISPLAY)printf("intnum%02d  begin:%5d end:%5d len:%5d (code:%2d)  real upper boundary: real:%5d\n",i,lboundary,intervalsize(i)+lboundary,intervalsize(i),intervals[i],boundary);
	    lboundary+=intervalsize(i);
	}
	intervals[i]=1+log2(sorted[n-1]-lboundary)+1;
	lboundary+=intervalsize(i);
	if(TMPDISPLAY)printf("num%02d  interval len:%2d  %5d   boundary: real:%5d approx:%5d\n",i,intervals[i],intervalsize(i),sorted[n-1],lboundary);
	if(TMPDISPLAY)printf("\n");
	delete [] sorted;
    }
};


// **************************************************
// *************** BitStream  ***********************
// **************************************************


int 
BitStream::check_tag(char *tag,int pos/*=-1*/)
{
    if(!use_tags){return OK;}
    if(!tag){return OK;}
    int found=-1;
    int ok=0;
    if(pos==-1){pos=bitpos;}
    for(int i=0;i<tags.size();i++)
    {
	if(!strcmp(tags[i],tag))
	{
	    found=tagpos[i];
	    if(tagpos[i]==pos){ok=1;break;}
	}
    }
    if(!ok)
    {
	show();
	if(found>=0)
	{
	    printf("ERROR:BitStream:bitpos:%4d:check_tag: found tag %s at %d expected it at %d\n",bitpos,tag,found,pos);
	}
	else
	{
	    printf("ERROR:BitStream:bitpos:%4d:check_tag:  tag %s not found, expected it at %d\n",bitpos,tag,pos);
	}
	return(NOTOK);
    }
    return(OK);
}

int 
BitStream::find_tag(char *tag)
{
    int i;
    for(i=0;i<tags.size() && strcmp(tag,tags[i]);i++);
    if(i==tags.size()){return -1;}
    else{return i;}
}
int 
BitStream::find_tag(int pos,int posaftertag/*=1*/)
{
    int i;
    for(i=0;i<tags.size() && tagpos[i]<pos;i++);
    if(i==tags.size()){return -1;}
    if(!posaftertag){return i;}
    for(;tagpos[i]>pos && i>=0;i--);
    return(i);	
}

void 
BitStream::show_bits(int a,int n)
{
    for(int b=a;b<a+n;b++)
    {
	printf("%c",(buff[b/8] & (1<<(b%8)) ? '1' : '0'));
    }
}
void 
BitStream::show(int a/*=0*/,int n/*=-1*/)
{
    int all=(n<0 ? 1 : 0);
    if(n<0){n=bitpos-a;}
    int i;

    if(all)
    {
	printf("BitStream::Show: ntags:%d size:%4d buffsize:%6d ::: ",tags.size(),size(),buffsize());
//  	    for(i=0;i<tags.size();i++){printf("tag:%d:%s:pos:%d\n",i,tags[i],tagpos[i]);}
    }

    int t=find_tag(a,0);
    if(t<0){show_bits(a,n);return;}
    for(i=a;i<a+n;i++)
    {
	for(;t<tags.size() && tagpos[t]<i+1;t++)
	{
	    printf("# %s:%03d:%03d #",tags[t],tagpos[t],n);
	}
	show_bits(i,1);
    }
    if(all){printf("\n");}

}



// **************************************************
// *************** Compressor ***********************
// **************************************************


int 
Compressor::put_vals(unsigned int *vals,int n,char *tag)
{
    int cpos=bitpos;
    add_tag(tag);
    if(n>=pow2(NBITS_NVALS)){cerr << "Compressor::put(uint *,nvals) : overflow: nvals>2^16" <<endl;fatal;}
    put(n,NBITS_NVALS,"size");
    if(n==0){return NBITS_NVALS;}


    freeze();
    compress_decr(vals,n);
    int sdecr=unfreeze();

    freeze();
    compress_fixed(vals,n);
    int sfixed=unfreeze();

    if(sdecr<sfixed)
    {
	put(0,2,"put_valsCompType");
	compress_decr(vals,n);
    }
    else
    {
	put(1,2,"put_valsCompType");
	compress_fixed(vals,n);
    }

    return(bitpos-cpos);
}

int 
Compressor::get_vals(unsigned int **pres,char *tag="BADTAG!")
{
    if(check_tag(tag)==NOTOK){fatal;return -1;}
    int n=get(NBITS_NVALS);
    if(TMPDISPLAY)printf("get_vals n:%d\n",n);
    if(!n){*pres=NULL;return 0;}
    unsigned int *res=new unsigned int[n];
    CHECK_MEM(res);


    int comptype=get(2,"put_valsCompType");
    switch(comptype)
    {
    case 0:    get_decr(res,n);
break;
    case 1:    get_fixedbitl(res,n);
break;
    default:   errr("Compressor::get_vals invalid comptype");break;
    }
//      get_fixedbitl(res,n);
//      get_decr(res,n);

    *pres=res;
    return(n);
}


int 
Compressor::put_fixedbitl(byte *vals,int n,char *tag)
{
    int cpos=bitpos;
    int i,j;
    add_tag(tag);
    byte maxv=vals[0];
    for(i=1;i<n;i++)
    {
	byte v=vals[i];
	if(v>maxv){maxv=v;}
    }
    int nbits=num_bits(maxv);
    if(n>=pow2(NBITS_NVALS)){cerr << "Compressor::put_fixedbitl(byte *) : overflow: nvals>2^16" <<endl;fatal;}
    put(n,NBITS_NVALS,"size");
    if(n==0){return 0;}
    put(nbits,4,"nbits");
    add_tag("data");
    for(i=0;i<n;i++)
    {
	byte v=vals[i];
	for(j=0;j<nbits;j++) {put(v&pow2(j));}
    }	
    return(bitpos-cpos);
}
void
Compressor::compress_fixed(unsigned int *vals,int n)
{
    int nbits=num_bits(max_v(vals,n));

    put(nbits,4,"nbits");
    add_tag("data");

    for(int i=0;i<n;i++)
    {
	if(TMPDISPLAY){printf("AA##AA %4d\n",vals[i]);}
	put(vals[i],nbits,NULL);
    }	
}

void
Compressor::get_fixedbitl(unsigned int *res,int n)
{
    int nbits=get(4);
//	printf("get_fixedbitl(uint):n%3d nbits:%2d\n",n,nbits);
    int i;
    for(i=0;i<n;i++)
    {
	res[i]=get(nbits);
    }
}
int 
Compressor::get_fixedbitl(byte **pres,char *tag/*="BADTAG!"*/)
{
    if(check_tag(tag)==NOTOK){fatal;return -1;}
    int n=get(NBITS_NVALS);
    if(!n){*pres=NULL;return 0;}
    int nbits=get(4);
//	printf("get_fixedbitl(byte):n%3d nbits:%2d\n",n,nbits);
    int i;
    byte *res=new byte[n];
    CHECK_MEM(res);
    for(i=0;i<n;i++)
    {
	res[i]=get(nbits);
    }
    *pres=res;
    return(n);
}

void
Compressor::compress_decr(unsigned int *vals,int n)
{
    VlengthCoder coder(vals,n,*this);
    coder.code_begin();
    int i;
    for(i=0;i<n;i++){coder.code(vals[i]);}
}
void
Compressor::get_decr(unsigned int *res,int n)
{
    VlengthCoder coder(*this);
    coder.get_begin();
    int i;
    for(i=0;i<n;i++){res[i]=coder.get();}
}
