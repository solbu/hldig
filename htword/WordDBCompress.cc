

#include "WordDBCompress.h"
#include "WordBitCompress.h"
#include "WordRecord.h"


// never change NBITS_COMPRESS_VERSION ! (otherwise version tracking will fail)
#define NBITS_COMPRESS_VERSION 10

// IMPORTANT: change these EVERY time you change something that affects the compression
#define COMPRESS_VERSION 3
char *version_label[]={"INVALID_VERSION_0","INVALID_VERSION_1","INVALID_VERSION_2","14 Dec 1999",NULL};

char *
get_version_label(int v)
{
    if(COMPRESS_VERSION <0 || COMPRESS_VERSION>((sizeof(version_label)/sizeof(*version_label))-1))
    {
	errr("get_version_label: version_label[COMPRESS_VERSION] is not valid, please update version_label");
    }
    if( v >= (int)((sizeof(version_label)/sizeof(*version_label))-1) )
    {
	return("INVALID_VERSION");
    }
    return(version_label[v]);
}
#define NBITS_CMPRTYPE 2
#define CMPRTYPE_NORMALCOMRPESS 0
#define CMPRTYPE_BADCOMPRESS 1

//byte a;
#define allign(v,a) ( (v)%(a) ? (v+((a)-(v)%(a))) : v)
#define NBITS_KEYLEN 16
#define NBITS_DATALEN 16



// ***********************************************
// *************** WordDBRecord  *****************
// ***********************************************

class WordDBRecord : public WordRecord
{
public:
    int set_decompress(unsigned int **data,int *indexes,int i,int pdata,int pstat0,int pstat1)
    {
	int datatype;
	if(i>=indexes[pstat0])
	{
	    datatype=1;
	    type=(datatype ? WORD_RECORD_DATA : WORD_RECORD_STATS);
	    info.data=data[pdata][i-indexes[pstat0]];
	}
	else
	{
	    datatype=0;
	    type=(datatype ? WORD_RECORD_DATA : WORD_RECORD_STATS);
	    info.stats.noccurence=data[pstat0][i];
	    info.stats.ndoc      =data[pstat1][i];
	}
	return(datatype);
    }
    WordDBRecord():WordRecord()
    {
;
    }
    WordDBRecord(byte *dat,int len,int rectyp):WordRecord()
    {
	type=(rectyp ? WORD_RECORD_DATA : WORD_RECORD_STATS);
	Unpack(String((char *)dat,len));
    }
    WordDBRecord(BKEYDATA *ndata,int rectyp):WordRecord()
    {// typ: 0->stat 1->data
	type=(rectyp ? WORD_RECORD_DATA : WORD_RECORD_STATS);
	Unpack(String((char *)ndata->data,ndata->len));
    }
};


// ***********************************************
// ****************  WordDBKey   *****************
// ***********************************************

class WordDBKey : public WordKey
{
    BKEYDATA *key;
public:
//      String word;
    int nbytes;
    u_int8_t bytes(int i){return(key->data[nbytes-i-1]);}

    int RecType(){return (GetWord()[0]!=1 ? 1 :0);}
    WordDBKey():WordKey()
    {
	nbytes=word_key_info.sort[0].bytes_offset;
	key=NULL;
    }
    WordDBKey(BKEYDATA *nkey):WordKey()
    {
	nbytes=word_key_info.sort[0].bytes_offset;
	key=nkey;
	Unpack(String((char *)key->data,key->len));
    }
    int is_null()
    {
	errr("UNUSED");
  	if(GetWord().length()==0)
	{
	    for(int j=1;j<word_key_info.nfields;j++)
	    {if(GetInSortOrder(j)!=0){errr("WordDBKey::is_null key has 0 len word but is not null");}}
	    return 1;
	}
	return 0;
    }
    WordDBKey(BINTERNAL *nkey):WordKey()
    {
	nbytes=word_key_info.sort[0].bytes_offset;
	key=NULL;
	if(nkey->len==0)
	{
	    ;//	    errr("WordDBKey::WordDBKey(BINTERNAL) : nkey->len==0");
	}
	else{Unpack(String((char *)nkey->data,nkey->len));}
    }
    WordDBKey(byte *data,int len):WordKey()
    {
	nbytes=word_key_info.sort[0].bytes_offset;
	key=NULL;
	if(!data || !len){errr("WordDBKey::WordDBKey(data,len) !data || !len");}
	Unpack(String((char *)data,len));
    }
//      WordDBKey(){;}
};



// ***********************************************
// ****************  WordDBPage  *****************
// ***********************************************

class WordDBPage
{
 public:
    int n;   // number of entries
    int nk;  // number of keys
    int type; // for now 3(btreeinternal) && 5(leave:normal case) are allowed
    int decmpr_pos;
    int decmpr_indx;
    PAGE *pg;


    void isleave()
    {
	if(type!=5){errr("WordDBPage::isleave: trying leave specific on non leave");}
    }
    void isintern()
    {
	if(type!=3){errr("WordDBPage::isintern: trying btreeinternal  specific on non btreeinternal page type");}

    }
    WordDBKey get_WordDBKey(int i)
    {
	if(type==5){return(WordDBKey(key(i)));}
	else
	if(type==3){return(WordDBKey(btikey(i)));}
	else
	{errr("WordDBPage:get_WordDBKey: bad page type");}
	return WordDBKey();
    }
    BINTERNAL *btikey(int i)
    {
	if(i<0 || i>=pg->entries){printf("btikey:%d\n",i);errr("WordDBPage::btikey out iof bounds");}
	isintern();return(GET_BINTERNAL(pg,i    ));
    }
    BKEYDATA  *entry (int i)
    {
	if(i<0 || i>=pg->entries){printf("entry:%d\n",i);errr("WordDBPage::entry out iof bounds");}
	isleave(); return(GET_BKEYDATA (pg,i    ));
    }
    BKEYDATA  *key   (int i)
    {
	if(i<0 || 2*i>=pg->entries){printf("key:%d\n",i);errr("WordDBPage::key out iof bounds");}
	isleave(); return(GET_BKEYDATA (pg,i*2  ));
    }
    BKEYDATA  *data  (int i)
    {
	if(i<0 || 2*i+1>=pg->entries){printf("data:%d\n",i);errr("WordDBPage::data out iof bounds");}
	isleave(); return(GET_BKEYDATA (pg,i*2+1));
    }
    int e_offset(int i) {return((int)(pg->inp[i]));}
    void *alloc_entry(int size)
    {
	size=allign(size,4);	
	int inp_pos=((byte *)&(pg->inp[decmpr_indx]))-(byte *)pg;
	decmpr_pos-=size;
	if(decmpr_pos<=inp_pos)
	{
	    show();
	    printf("alloc_entry: allocating size:%4d entrynum:decmpr_indx:%4d at:decmpr_pos:%4d\n",size,decmpr_indx,decmpr_pos);
	    errr("WordDBPage::alloc_entry: PAGE OVERFLOW");
	}
	pg->inp[decmpr_indx++]=decmpr_pos;
	return((void *)((byte *)pg+decmpr_pos));
    }
    void insert_data(WordDBRecord &wrec)
    {
	isleave();
	if(!(decmpr_indx%2)){errr("WordDBPage::insert_data data must be an odd number!");}
	String prec;
	wrec.Pack(prec);
	int len=prec.length();
	int size=len+(sizeof(BKEYDATA)-1);
	
	BKEYDATA *dat=(BKEYDATA *)alloc_entry(size);
	dat->len=len;
	dat->type=1;
	memcpy((void *)dat->data,(void *)(char *)prec,len);
    }
    void insert_key(WordDBKey &ky)
    {
	isleave();
	if(decmpr_indx%2){errr("WordDBPage::insert_key key must be an even number!");}
	String pkey;
	ky.Pack(pkey);
	int keylen=pkey.length();
	int size=keylen+(sizeof(BKEYDATA)-1);
	BKEYDATA *bky=(BKEYDATA *)alloc_entry(size);
	bky->len=keylen;
	bky->type=1;// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	memcpy((void *)bky->data,(void *)(char *)pkey,keylen);
    }
    void insert_btikey(WordDBKey &ky,BINTERNAL &bti,int empty=0)
    {
	isintern();
	int keylen=0;
	String pkey;
	if(!empty)
	{
	    ky.Pack(pkey);
	    keylen=pkey.length();
	}
	int size=keylen+((byte *)&(bti.data))-((byte *)&bti);// pos of data field in BINTERNAL
	if(empty)
	{
	    if(verbose){printf("WordDBPage::insert_btikey: empty : BINTERNAL:%d datapos:%d keylen:%d size:%d alligned to:%d\n",sizeof(BINTERNAL),
			       ((byte *)&(bti.data))-((byte *)&bti),keylen,size,allign(size,4));}
	}

	BINTERNAL *btik=(BINTERNAL *)alloc_entry(size);
	btik->len  =(empty ? 0 : keylen);
	btik->type=1;// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	btik->pgno =bti.pgno;
	btik->nrecs=bti.nrecs;
	if(!empty){memcpy((void *)btik->data,(void *)(char *)pkey,keylen);}
//  	else
//  	{btik->data[0]=0;}// just to avoid uninit memory read
    }
    int entry_struct_size()
    {
	return(type==3 ? sizeof(BINTERNAL) : sizeof(BKEYDATA )   )-1; 
    }
    int entry_size(int i)
    {
	return entry_struct_size() + (type==3 ? btikey(i)->len : key(i)->len   ); 
    }

    void compress_key(Compressor &out,int i)
    {
	if(type==3)
	{
	    int len=btikey(i)->len;
	    out.put(len,NBITS_KEYLEN,label_str("seperatekey_len",i));
	    if(verbose){printf("WordDBPage::compress_key:compress(typ3):%d ::: sizeof(BINTERNAL):%d\n",len,sizeof(BINTERNAL));}
	    out.put(btikey(i)->len  ,sizeof(btikey(i)->len  )*8,label_str("seperatekey_bti_len"  ,i));
	    out.put(btikey(i)->type ,sizeof(btikey(i)->type )*8,label_str("seperatekey_bti_type" ,i));
	    out.put(btikey(i)->pgno ,sizeof(btikey(i)->pgno )*8,label_str("seperatekey_bti_pgno" ,i));
	    out.put(btikey(i)->nrecs,sizeof(btikey(i)->nrecs)*8,label_str("seperatekey_bti_nrecs",i));
	    if(len){out.put_zone((byte *)btikey(i)->data,8*len,label_str("seperatekey_btidata",i));}
	}
	else
	{
	    int len=key(i)->len;
	    out.put(len,NBITS_KEYLEN,label_str("seperatekey_len",i));
	    if(verbose){printf("WordDBPage::compress_key: compress(typ5):%d\n",len);}
	    out.put_zone((byte *)key(i)->data,8*len,label_str("seperatekey_data",i));
	}
    }
    void compress_data(Compressor &out,int i)
    {
	int len=data(i)->len;
	out.put(len,NBITS_DATALEN,label_str("seperatedata_len",i));
	if(verbose){printf("WordDBPage::compress_data: compressdata(typ5):%d\n",len);}
	out.put_zone((byte *)data(i)->data,8*len,label_str("seperatedata_data",i));
    }
    WordDBKey uncompress_key(Compressor &in,int i)
    {
	WordDBKey res;
	int len=in.get(NBITS_KEYLEN,label_str("seperatekey_len",i));
	if(verbose){printf("WordDBPage::uncompress_key: seperatekey:len:%d\n",len);}
	
	if(type==3)
	{
	    if(len==0 && i!=0){errr("WordDBPage::uncompress_key: keylen=0 &&    i!=0");}
	    BINTERNAL bti;
	    bti.len  =in.get(sizeof(bti.len  )*8,label_str("seperatekey_bti_len"  ,i));
	    bti.type =in.get(sizeof(bti.type )*8,label_str("seperatekey_bti_type" ,i));
	    bti.pgno =in.get(sizeof(bti.pgno )*8,label_str("seperatekey_bti_pgno" ,i));
	    bti.nrecs=in.get(sizeof(bti.nrecs)*8,label_str("seperatekey_bti_nrecs",i));
	    if(len!=bti.len){errr("WordDBPage::uncompress_key: incoherence: len!=bti.len");}
	    if(len)
	    {
		byte *gotdata=new byte[len];
		CHECK_MEM(gotdata);
		in.get_zone(gotdata,8*len,label_str("seperatekey_btidata",i));
		res=WordDBKey(gotdata,len);
		delete [] gotdata;
	    }
	    insert_btikey(res,bti,(len==0 ? 1:0));
	}
	else
	{
	    byte *gotdata=new byte[len];
	    CHECK_MEM(gotdata);
	    in.get_zone(gotdata,8*len,label_str("seperatekey_data",i));
	    res=WordDBKey(gotdata,len);
	    insert_key(res);
	    delete [] gotdata;
	}
	return res;
    }
    WordDBRecord uncompress_data(Compressor &in,int i,int rectyp)
    {
	WordDBRecord res;
	int len=in.get(NBITS_DATALEN,label_str("seperatedata_len",i));
	if(verbose)printf("uncompressdata:len:%d\n",len);
	byte *gotdata=new byte[len];
	CHECK_MEM(gotdata);
	in.get_zone(gotdata,8*len,label_str("seperatedata_data",i));
	res=WordDBRecord(gotdata,len,rectyp);
	insert_data(res);
	delete [] gotdata;
	return res;
    }
    int pgsz;
    void show(int redo=0);

    int TestCompress(int debuglevel);
    int Compare(WordDBPage &other);
    
    int verbose;
    int debug;
    
    void Compress_extract_vals_wordiffs(int *nums,int *nums_pos,int nnums,HtVector_byte &wordiffs);
    void Compress_show_extracted(int *nums,int *nums_pos,int nnums,HtVector_byte &wordiffs);
    void Compress_vals(Compressor &out,int *nums,int *nums_pos,int nnums);
    void Compress_header(Compressor &out);
    int  Compress_main(Compressor &out);
    Compressor *Compress(int debug=0);

    int  Uncompress(Compressor *pin,int debug=0);
    int  Uncompress_main(Compressor *pin);
    int  Uncompress_header(Compressor &in);
    void Uncompress_rebuild(Compressor &in,unsigned int **rnums,int *rnum_sizes,int nnums,byte *rworddiffs,int nrworddiffs);
    void Uncompress_show_rebuild(unsigned int **rnums,int *rnum_sizes,int nnums,byte *rworddiffs,int nrworddiffs);

    // initialize when header is valid
    void init() 
    {
	type=pg->type;
	n=pg->entries;
	nk=(type==5 ? n/2 : n);
	decmpr_pos=pgsz;
	decmpr_indx=0;
    }


    int CNFLAGS        ;
    int CNFIELDS       ;
    int CNDATASTATS0   ;
    int CNDATASTATS1   ;
    int CNDATADATA     ;
    int CNBTIPGNO      ;
    int CNBTINRECS     ;
    int CNWORDDIFFPOS  ;
    int CNWORDDIFFLEN  ;
    int nnums      ;
    void init0()
    {
	CNFLAGS        =0;
	CNFIELDS       =1;
	CNDATASTATS0   = word_key_info.nfields    ;
	CNDATASTATS1   = word_key_info.nfields + 1;
	CNDATADATA     = word_key_info.nfields + 2;
	CNBTIPGNO      = word_key_info.nfields + 3;
	CNBTINRECS     = word_key_info.nfields + 4;
	CNWORDDIFFPOS  = word_key_info.nfields + 5;
	CNWORDDIFFLEN  = word_key_info.nfields + 6;
	nnums=(CNWORDDIFFLEN+1);

	pg=NULL;
	pgsz=0;
	n=0;
	nk=0;
	type=-1;
	verbose=0;
	debug=0;
	decmpr_pos=pgsz;
	decmpr_indx=0;
    }

    void delete_page()
    {
       if(!pg){errr("WordDBPage::delete_page: pg==NULL");}
       delete [] pg;
       pg=NULL;
    }
    void unset_page()
    {
       if(!pg){errr("WordDBPage::unset_page: pg==NULL");}
       pg=NULL;
    }
    ~WordDBPage()
    {
	if(pg){errr("WordDBPage::~WordDBPage: page not empty");}
    }
    WordDBPage(int npgsz)
    {
	init0();
	pgsz=npgsz;
	pg=(PAGE *)(new byte[pgsz]);
	CHECK_MEM(pg);
	decmpr_pos=pgsz;
	decmpr_indx=0;
    }
    WordDBPage(const u_int8_t* buff,int buff_length)
    {
	init0();
	pg=(PAGE *)buff;
	pgsz=buff_length;
	decmpr_pos=pgsz;
	decmpr_indx=0;
	init();
    }
};







// ***********************************************
// *********** WordDBCompress  *******************
// ***********************************************

//word_key_info.sort[position].encoding_position

WordDBCompress::WordDBCompress()
{
    debug=2;
}

extern "C"
{
extern int __memp_cmpr_inflate (const u_int8_t *, int, u_int8_t *, int, void *);
extern int __memp_cmpr_deflate (const u_int8_t *, int, u_int8_t **, int*, void *);
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

int cmprcount=0;
//  Compresses inbuff to outbuff
int 
WordDBCompress::Compress(const  u_int8_t *inbuff, int inbuff_length, u_int8_t **outbuffp, int *outbuff_lengthp)
{
    // create a page from inbuff
    WordDBPage pg(inbuff,inbuff_length);

    if(debug>2)
    {
	printf("###########################  WordDBCompress::Compress:%5d  #################################################\n",cmprcount);
	pg.show(3);
	printf("~~~~~~~~~~~~~\n");
    }


//      pg.show();
    // DEBUG: check if decompressed compresed page is equivalent to original
    if(debug)TestCompress(inbuff,inbuff_length,debug);

    // do the real compression
    Compressor *res=pg.Compress(0);

    // copy it to outbuff
    (*outbuffp)=res->get_data();
    (*outbuff_lengthp)=res->buffsize();





    if(debug>2)
    {
	res->show();
	printf("\n%%%%%%%% Final COMPRESSED size:%4d   %f\n",res->size(),res->size()/8.0);
	printf("***************************  %5d  #################################################\n",cmprcount++);
    }

    delete res;
    if(debug>2){printf("WordDBCompress::Compress: final output size:%6d\n",(*outbuff_lengthp));}

    // cleanup
    pg.unset_page();
    return(0);
}

//  Uncompresses inbuff to outbuff
int 
WordDBCompress::Uncompress(const u_int8_t *inbuff, int inbuff_length, u_int8_t *outbuff,int outbuff_length)
{
    if(debug>2){printf("WordDBCompress::Uncompress::  %5d -> %5d\n",inbuff_length,outbuff_length);}
    // create a page for decompressing into it
    WordDBPage pg(outbuff_length);
    if(debug>2){printf("------------------------  WordDBCompress::Uncompress:%5d --------------------------------\n",cmprcount);}

    // create a Compressor from inbuff and setit up
    Compressor in(inbuff_length);
    in.set_data(inbuff,inbuff_length*8);
    in.rewind();

    // do the uncompression
    pg.Uncompress(&in,0);
    
    // copy the result to outbuff
    memcpy((void *)outbuff,(void *)pg.pg,outbuff_length);

    pg.delete_page();
    if(debug>2){printf("------------------------  WordDBCompress::Uncompress: END %d\n",cmprcount);}
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




// ***********************************************
// **********  WordDBPage  ***********************
// ***********************************************


// checks if compression/decompression sequence is harmless
int
WordDBPage::TestCompress(int debuglevel)
{
    if(debuglevel>2){printf("ttttttttttttt WordDBPage::TestCompress  BEGIN\n");}
    int compress_debug=debuglevel-1;
//      printf("WordDBPage::TestCompress: \n");
    // start by compressing this page
    Compressor *res=Compress(compress_debug);
//      printf("WordDBPage::TestCompress: res:%x show:\n",res);
//      res->show();

    if(res)
    {
	int size=res->size();
	// now uncompress into pageu
	WordDBPage pageu(pgsz);
	res->rewind();
	pageu.Uncompress(res,compress_debug);
	
	// comapre this page and pageu
	int cmp=Compare(pageu);

	// show some results
  	if(debuglevel>2)printf("TOTAL SIZE: %6d %8f\n",size,size/8.0);
	// argh! compare failed somthing went wrong
	// display the compress/decompress sequence and fail
	if(cmp || size>8*1024*1000000000)
	{
	    if(size>8*1024)
	    {
		printf("---------------------------------------------------\n");
		printf("-----------overflow:%5d------------------------------\n",size/8);
		printf("---------------------------------------------------\n");
		printf("---------------------------------------------------\n");
	    }
	    printf("###################  ORIGINAL #########################################\n");
	    show();
	    printf("###################  REDECOMPRESSED #########################################\n");
	    pageu.show();
	    
	    // re-compress the page verbosely
	    Compressor *res2=Compress(2);
	    res2->rewind();
	    // re-uncompress the page verbosely
	    WordDBPage pageu2(pgsz);
	    pageu2.Uncompress(res2,2);
	    pageu2.show();
	    if(cmp){errr("Compare failed");}
	    delete res2;
	}
	pageu.delete_page();
	delete res;

    }else {errr("WordDBPage::TestCompress: Compress failed");}

    if(debuglevel>2){printf("ttttttttttttt WordDBPage::TestCompress  END\n");}
    return OK;
}

// find position of first difference between 2 strings
int first_diff(const String &s1,const String &s2)
{
    int j;
    for(j=0;j<s1.length() && j<s2.length() && s1[j]==s2[j];j++);
    return(j);
}



// ******* Uncompress Compressor into this page
int 
WordDBPage::Uncompress(Compressor *pin,int  ndebug)
{
//      printf("WordDBPage::Uncompress:pin->size:%d pin->buffsize:%d\n",pin->size(),pin->buffsize());
//      pin->show();
//      printf("WordDBPage::Uncompress after show\n");
    debug=ndebug;
    if(debug>1){verbose=1;}
    if(verbose){printf("uuuuuuuuu WordDBPage::Uncompress: BEGIN\n");}
    
    int read_version = pin->get(NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
    if(read_version != COMPRESS_VERSION)
    {
	fprintf(stderr,"WordDBPage::Uncompress: ***        Compression version mismatch      ***\n");
	fprintf(stderr,"found version      : %3d     but using version : %3d\n",read_version,COMPRESS_VERSION);
	fprintf(stderr,"found version label: %s\n",get_version_label(read_version));
	fprintf(stderr,"using version label: %s\n",get_version_label(COMPRESS_VERSION));
	fprintf(stderr,"Are you sure you're not reading an old DB with a newer version of the indexer??\n");
	errr("WordDBPage::Uncompress: ***        Compression version mismatch      ***");
	exit(1);
    }
    int cmprtype=pin->get(NBITS_CMPRTYPE,"CMPRTYPE");   
    // two possible cases
    switch(cmprtype)
    {
    case CMPRTYPE_NORMALCOMRPESS:// this was a normaly compressed page
	Uncompress_main(pin);
	break;
    case CMPRTYPE_BADCOMPRESS:// this page did not compress correctly
//  	delete [] pg;
	pin->get_zone((byte *)pg,pgsz*8,"INITIALBUFFER");
//  	show();
	break;
    default:
	errr("WordDBPage::Uncompress: CMPRTYPE incoherent");
    }

    if(verbose){printf("uuuuuuuuu WordDBPage::Uncompress: END\n");}
    return OK;
}

// ******* Uncompress Compressor into this page
// normally compressed page case
int 
WordDBPage::Uncompress_main(Compressor *pin)
{
    if(!pin){errr("WordDBPage::Uncompress: no Compressor to uncompress from!!");}
    Compressor &in=*((Compressor *)pin);
    if(debug>0){in.set_use_tags();}
    int i,j;
    unsigned int **rnums=new (unsigned int *)[nnums];
    CHECK_MEM(rnums);
    int *rnum_sizes=new int[nnums];
    CHECK_MEM(rnum_sizes);
    byte *rworddiffs=NULL;
    int nrworddiffs;

//      int this_is_temporary=0;
//    memset((void *)pg,0,pgsz);
    
    // *********** read header
    if(Uncompress_header(in)!=OK){return NOTOK;}

    // get first key(s):
    //type=5: key(0) stored seperately ... others are decompressed frome differences
    // 
    //type=3: btikey(0) is particular (len=0) it is stored seperately
    //        btikey(1) stored seperately ... others are decompressed frome differences
    //
    int nkeysleft=nk;
    if(nkeysleft>0)
    {
	WordDBKey key0=uncompress_key(in,0);
	if(type==5){uncompress_data(in,0,key0.RecType());}
	nkeysleft--;
    }
    if(nkeysleft>0 && type==3){uncompress_key(in,1);nkeysleft--;}

    if(nkeysleft>0)
    {
	// ********* read numerical fields
	for(j=0;j<nnums;j++)
	{
	    if(verbose)printf("field %2d : start position:%4d  \n",j,in.size());
	    if(j==3 && verbose){in.verbose=2;}
	    rnum_sizes[j]=in.get_vals(&(rnums[j]),label_str("NumField",j));//***
	    if(j==3 && verbose){in.verbose=0;}
	    if(verbose){printf("WordDBPage::Uncompress_main:got numfield:%2d:nvals:%4d\n",j,rnum_sizes[j]);}
	}

	//  ********* read word differences
	nrworddiffs=in.get_fixedbitl(&rworddiffs,"WordDiffs");


	//  ********* rebuild original page
	Uncompress_rebuild(in,rnums,rnum_sizes,nnums,rworddiffs,nrworddiffs);
	Uncompress_show_rebuild(rnums,rnum_sizes,nnums,rworddiffs,nrworddiffs);


	for(i=0;i<nnums;i++){delete [] rnums[i];}
    }
    delete [] rnums;
    delete [] rnum_sizes;
    if(rworddiffs){delete [] rworddiffs;}
    return 0;
}
int 
WordDBPage::Uncompress_header(Compressor &in)
{
    pg->lsn.file     =in.get(  8*sizeof(pg->lsn.file    ),"page:lsn.file");
    pg->lsn.offset   =in.get(  8*sizeof(pg->lsn.offset  ),"page:lsn.offset");
    pg->pgno         =in.get(  8*sizeof(pg->pgno        ),"page:pgno");
    pg->prev_pgno    =in.get(  8*sizeof(pg->prev_pgno   ),"page:prev_pgno");
    pg->next_pgno    =in.get(  8*sizeof(pg->next_pgno   ),"page:next_pgno");
    pg->entries      =in.get(  8*sizeof(pg->entries     ),"page:entries");
    pg->hf_offset    =in.get(  8*sizeof(pg->hf_offset   ),"page:hf_offset");
    pg->level        =in.get(  8*sizeof(pg->level       ),"page:level");
    pg->type         =in.get(  8*sizeof(pg->type        ),"page:type");

    init();

    if(verbose)
    {
	printf("************************************\n");
	printf("********   WordDBPage::Uncompress: page header ***\n");
	printf("************************************\n");
	printf("page size:%d\n",(int)pgsz);
	printf(" 00-07: Log sequence number.  file  : %d\n",                            pg->lsn.file            );      
	printf(" 00-07: Log sequence number.  offset: %d\n",                            pg->lsn.offset            );      
	printf(" 08-11: Current page number.  : %d\n",		               pg->pgno            );     
	printf(" 12-15: Previous page number. : %d\n",		               pg->prev_pgno         );
	printf(" 16-19: Next page number.     : %d\n",			       pg->next_pgno           );
	printf(" 20-21: Number of item pairs on the page. : %d\n",	               pg->entries           );  
	printf(" 22-23: High free byte page offset.       : %d\n",	               pg->hf_offset        );
	printf("    24: Btree tree level.                 : %d\n",                pg->level             );	
	printf("    25: Page type.                        : %d\n",                pg->type               );		
    }
    return OK;
}
void 
WordDBPage::Uncompress_rebuild(Compressor &in,unsigned int **rnums,int *rnum_sizes,int nnums,byte *rworddiffs,int nrworddiffs)
{
    int irwordiffs=0;
    int nfields=word_key_info.nfields;
    int *rnum_pos=new int[   nnums];// current index count
    CHECK_MEM(rnum_pos);

    int ii,j;
    for(j=0;j<nnums;j++){rnum_pos[j]=0;}

    int i0=0;
    if(type==3){i0=1;}// internal pages have particular first key

    WordDBKey pkey;
    WordDBKey akey=get_WordDBKey(i0);

    for(ii=i0;ii<nk;ii++)
    {
	WordDBRecord arec;
	BINTERNAL bti;

	if(type==5)
	{
	    // **** get the data fields
	    arec.set_decompress(rnums,rnum_sizes,ii,CNDATADATA,CNDATASTATS0,CNDATASTATS1);
	}
	else
	{
	    if(type!=3){errr("WordDBPage::Uncompress_rebuild: unsupported type!=3");}
	    // ****** btree internal page specific
	    bti.pgno =rnums[CNBTIPGNO ][rnum_pos[CNBTIPGNO ]++];
	    bti.nrecs=rnums[CNBTINRECS][rnum_pos[CNBTINRECS]++];
	}
	// all that follows codes differences between succesive entries
	// that is: Numerical key fields, Words
	if(ii>i0)
	{
	    unsigned int flags=rnums[CNFLAGS][rnum_pos[CNFLAGS]++];
	    int foundfchange=0;
	    // **** get the word
//  	int nrworddiffs=0;
	    if(flags&pow2(nfields-1))
	    {
		foundfchange=1;
		if(rnum_pos[CNWORDDIFFLEN]>=rnum_sizes[CNWORDDIFFLEN]){errr("WordDBPage::Uncompress read wrong num worddiffs");}
		int diffpos=rnums[CNWORDDIFFPOS][rnum_pos[CNWORDDIFFPOS]++];
		int difflen=rnums[CNWORDDIFFLEN][rnum_pos[CNWORDDIFFLEN]++];
		int wlen=diffpos+difflen;
		char *str=new char [wlen+1];
		CHECK_MEM(str);
		if(diffpos)strncpy(str,(char *)pkey.GetWord(),diffpos);
		strncpy(str+diffpos,(char *)rworddiffs+irwordiffs,difflen);
		str[wlen]=0;
		if(verbose)printf("key %3d word:\"%s\"\n",ii,str);
		akey.SetWord(str);
		irwordiffs+=difflen;
		delete [] str;
	    }else{akey.SetWord(pkey.GetWord());}
	    // **** get the numerical key fields
	    for(j=1;j<nfields;j++)
	    {
		int changed=flags&pow2(j-1);
		if(changed)
		{
		    int k=CNFIELDS+j-1;
		    int indx=rnum_pos[k];
		    if(indx>=rnum_sizes[k]){errr("WordDBPage::Uncompress read wrong num ogf changes in a field");}
		    if(!foundfchange)
		    {
			akey.SetInSortOrder(j,rnums[k][indx]+pkey.GetInSortOrder(j));
		    }
		    else
		    {
			akey.SetInSortOrder(j,rnums[k][indx]);
		    }
		    rnum_pos[k]++;
		    foundfchange=1;
		}
		else
		{
		    if(!foundfchange){akey.SetInSortOrder(j,pkey.GetInSortOrder(j));}
		    else{akey.SetInSortOrder(j,0);}
		}
	    }
	}
	// now insert key/data into page
	if(type==5)
	{
	    if(ii>i0)insert_key(akey);
	    if(ii>i0)insert_data(arec);
	}
	else
	{
	    if(type!=3){errr("WordDBPage::Uncompress_rebuild: unsupported type!=3");}
	    if(ii>i0)insert_btikey(akey,bti);
	}
	pkey=akey;
    }
    delete [] rnum_pos;
}

void 
WordDBPage::Uncompress_show_rebuild(unsigned int **rnums,int *rnum_sizes,int nnums,byte *rworddiffs,int nrworddiffs)
{
    int i,j;
    if(verbose)
    {
	printf("WordDBPage::Uncompress_show_rebuild: rebuilt numerical fields\n");
	for(j=0;j<nnums;j++)
	{
	    printf("resfield %2d:",j);
	    for(i=0;i<rnum_sizes[j];i++)
	    {
		printf("%4d ",rnums[j][i]);
	    }
	    printf("\n");
	    printf("diffield %2d:",j);
	    for(i=0;i<rnum_sizes[j];i++)
	    {
		;//		printf("%2d:%d ",i,nums[j*nk+i] == rnums[j][i]);		    
	    }
	    printf("\n");
	}
	printf("reswordiffs:");
	for(i=0;i<nrworddiffs;i++){printf("%c",(isalnum(rworddiffs[i]) ? rworddiffs[i] : '#'));}
	printf("\n");
    }
}
Compressor *
WordDBPage::Compress(int ndebug)
{
    debug=ndebug;
    if(debug>1){verbose=1;}

    Compressor *res=(Compressor *)new Compressor(pgsz);
    CHECK_MEM(res);
    if(debug>0){res->set_use_tags();}
    res->put(COMPRESS_VERSION,NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
    res->put(CMPRTYPE_NORMALCOMRPESS,NBITS_CMPRTYPE,"CMPRTYPE");
    if(verbose){printf("WordDBPage::Compress: trying normal compress\n");}
    int cmpr_ok=Compress_main(*((Compressor *)res));

    if(cmpr_ok!=OK || res->buffsize()>pgsz)
    {
    	if(verbose){printf("WordDBCompress::Compress full compress failed ... not compressing at all\n");}
  	show();
	if(res){delete res;}
	res=new Compressor;
	CHECK_MEM(res);
	if(debug>0){res->set_use_tags();}
	res->put(COMPRESS_VERSION,NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
	res->put(CMPRTYPE_BADCOMPRESS,NBITS_CMPRTYPE,"CMPRTYPE");
	res->put_zone((byte *)pg,pgsz*8,"INITIALBUFFER");
    }

    if(verbose)
    {
	printf("WordDBPage::Compress: Final bitstream result for %d\n",cmprcount);
	res->show();
    }
    return res;
};

int
WordDBPage::Compress_main(Compressor &out)
{
    if(debug>1){verbose=1;}
    if(verbose){printf("WordDBPage::Compress_main: starting compression\n");}
    
    if(pg->type!=5 && pg->type!=3){    printf("pg->type:%3d\n",pg->type);return NOTOK;}
//        if(pg->type==3){show();}


    // *************** initialize data structures **************
    int j;
    // 0 -> changed/unchanged flags   :  4bits
    // 1..n -> numerical fields delta :  ?bits (depending on field)
    // n+1 -> word changed size       :  1
    int *nums    =new int[nk*nnums];
    CHECK_MEM(nums);
    int *nums_pos=new int[   nnums];
    CHECK_MEM(nums_pos);
//      int *cnsizes =new int[   nnums];
    for(j=0;j<nnums;j++){nums_pos[j]=0;}
//      for(j=1;j<nfields;j++)  {cnsizes[j]=word_key_info.sort[j].bits;}
//      cnsizes[CNFLAGS]=4;
//      cnsizes[CNWORDDIFFPOS ]=8;
//      cnsizes[CNWORDDIFFLEN ]=8;
    HtVector_byte worddiffs;
    

    // *************** extract values and wordiffs **************
    if(nk>0)
    {
	Compress_extract_vals_wordiffs(nums,nums_pos,nnums,worddiffs);
	if(verbose)Compress_show_extracted(nums,nums_pos,nnums,worddiffs);
    }

    // *************** init compression **************

    Compress_header(out);

    // *************** compress  values and wordiffs **************

    // compress first key(s)
    int nkeysleft=nk;
    if(nkeysleft>0)
    {
	compress_key(out,0);
	if(type==5){compress_data(out,0);}
	nkeysleft--;
    }
    if(nkeysleft>0 && type==3){compress_key(out,1);nkeysleft--;}

    if(nkeysleft>0)
    {
	// compress values
	Compress_vals(out,nums,nums_pos,nnums);

	// compress worddiffs
	int size=out.put_fixedbitl(worddiffs.begin(),worddiffs.size(),"WordDiffs");
	if(verbose)printf("compressed wordiffs : %3d values: %4d bits %4f bytes\n",worddiffs.size(),size,size/8.0);
    }





    // *************** cleanup **************

    delete [] nums ;
    delete [] nums_pos;
//      delete [] cnsizes  ;

    return OK;
}
void 
WordDBPage::Compress_extract_vals_wordiffs(int *nums,int *nums_pos,int nnums,HtVector_byte &worddiffs)
{
    WordDBKey pkey;


    int ii,j;
    int i0=0;
    if(type==3){i0=1;}// internal pages have particular first key
    for(ii=i0;ii<nk;ii++)
    {
	WordDBKey akey=get_WordDBKey(ii);

	if(type==5)
	{
            // ****** WordRecord (data/stats)
	    // get word record
	    WordDBRecord arec(data(ii),akey.RecType());
	    // add record 
	    if(arec.type==WORD_RECORD_STATS)
	    {
		nums[CNDATASTATS0*nk+nums_pos[CNDATASTATS0]++]=arec.info.stats.noccurence;
		nums[CNDATASTATS1*nk+nums_pos[CNDATASTATS1]++]=arec.info.stats.ndoc;
	    }
	    else
	    {
		nums[CNDATADATA  *nk+nums_pos[CNDATADATA  ]++]=arec.info.data;
	    }
	}
	else
	{
	    if(type!=3){errr("WordDBPage::Compress_extract_vals_wordiffs: unsupported type!=3");}
            // ****** btree internal page specific
	    nums[CNBTIPGNO *nk+nums_pos[CNBTIPGNO ]++]=btikey(ii)->pgno ;
	    nums[CNBTINRECS*nk+nums_pos[CNBTINRECS]++]=btikey(ii)->nrecs;
	}

	// all that follows codes differences between succesive entries
	// that is: Numerical key fields, Words
	if(ii>i0)
	{
	    //  clear changed falgs
	    int iflag=CNFLAGS*nk+nums_pos[CNFLAGS]++;
	    nums[iflag]=0;

	    int foundfchange=0;
	    String &aword=akey.GetWord();
	    String &pword=pkey.GetWord();
	    if(!(aword==pword)){foundfchange=1;}

	    // check numerical fields for changes
	    // ********   sets CNFIELDS and some of CNFLAGS ************
	    for(j=1;j<word_key_info.nfields;j++)
	    {
		int diff=akey.GetInSortOrder(j)-(foundfchange ? 0 : pkey.GetInSortOrder(j));
		if(diff)
		{
		    foundfchange=1;
		    nums[iflag]|=pow2(j-1);
		    nums[      j*nk+nums_pos[j]++]=diff;
		}
	    }

	    // ************ check word for changes
	    // ********   sets CNWORDDIFFPOS CNWORDDIFFLEN and some of CNFLAGS ************
	    if(!(aword==pword))
	    {
		nums[iflag]|=pow2(word_key_info.nfields-1);
//  	    printf("wordchanged:%d %s\n",(1<<(CNWORD)),(char *)aword);
		int fd=first_diff(aword,pword);
		nums[CNWORDDIFFPOS*nk+nums_pos[CNWORDDIFFPOS]++]=fd;
		nums[CNWORDDIFFLEN*nk+nums_pos[CNWORDDIFFLEN]++]=aword.length()-fd;
		for(int s=fd;s<aword.length();s++){worddiffs.push_back(aword[s]);}
	    }
	}
	pkey=akey;
    }
//      nums_pos[CNFLAGS]=nk-1;

}
void 
WordDBPage::Compress_vals(Compressor &out,int *nums,int *nums_pos,int nnums)
{
    int j;
    for(j=0;j<nnums;j++)
    {
	int n=nums_pos[j];
	unsigned int *v=(unsigned int *)(nums+j*nk);
	if(j==3 && verbose){out.verbose=2;}
	int dud;
	for(int i=0;i<n;i++){dud=v[i]>1;}// make purify error if v unintialized
	int size=out.put_vals(v,n,label_str("NumField",j));
	if(j==3 && verbose){out.verbose=0;}
	if(verbose)printf("compressed field %2d : %3d values: %4d bits %8f bytes  : ended bit field pos:%6d\n",j,n,size,size/8.0,out.size());
    }
}
void
WordDBPage::Compress_header(Compressor &out)
{
// no smart compression ... for now
    out.put(pg->lsn.file     ,  8*sizeof(pg->lsn.file    ),"page:lsn.file");
    out.put(pg->lsn.offset   ,  8*sizeof(pg->lsn.offset  ),"page:lsn.offset");
    out.put(pg->pgno         ,  8*sizeof(pg->pgno        ),"page:pgno");
    out.put(pg->prev_pgno    ,  8*sizeof(pg->prev_pgno   ),"page:prev_pgno");
    out.put(pg->next_pgno    ,  8*sizeof(pg->next_pgno   ),"page:next_pgno");
    out.put(pg->entries      ,  8*sizeof(pg->entries     ),"page:entries");
    out.put(pg->hf_offset    ,  8*sizeof(pg->hf_offset   ),"page:hf_offset");
    out.put(pg->level        ,  8*sizeof(pg->level       ),"page:level");
    out.put(pg->type         ,  8*sizeof(pg->type        ),"page:type");
}

void 
WordDBPage::Compress_show_extracted(int *nums,int *nums_pos,int nnums,HtVector_byte &worddiffs)
{
    int i,j;
    int *cnindexe2=new int[   nnums];
    CHECK_MEM(cnindexe2);
    for(j=0;j<nnums;j++){cnindexe2[j]=0;}
    int w=0;
    for(i=0;i<nk;i++)
    {
	printf("%3d: ",i);
	for(j=0;j<nnums;j++)
	{
	    int k=cnindexe2[j]++;
	    int nbits=(j ? 16:4);// just to show the flags field...
	    if(k<nums_pos[j])
	    {
		int val=nums[j*nk+k];
		if(nbits<8){show_bits(val,nbits);printf(" ");}
		else
		{
		    printf("% 6d ",val);
		}
	    }
	    else
	    {
		if(nbits<8){printf("    ");}
		else
		{
		    printf("       ");
		}
	    }
	}
	if(w<worddiffs.size()){printf("   %02x %c ",worddiffs[w],(isalnum(worddiffs[w]) ? worddiffs[w] : '#'));}
	w++;
	printf("\n");
    }
    delete [] cnindexe2;
}

// Compare two pages to check if equal
int
WordDBPage::Compare(WordDBPage &other)
{
    int res=0;
    // Compare headers
    if(other.pgsz           != pgsz             ){res++;printf("compare failed for  pgsz                 \n");}    
    if(other.pg->lsn.file   != pg->lsn.file     ){res++;printf("compare failed for  pg->lsn.file         \n");}    
    if(other.pg->lsn.offset != pg->lsn.offset   ){res++;printf("compare failed for  pg->lsn.offset       \n");}    
    if(other.pg->pgno       != pg->pgno         ){res++;printf("compare failed for  pg->pgno             \n");}    
    if(other.pg->prev_pgno  != pg->prev_pgno    ){res++;printf("compare failed for  pg->prev_pgno        \n");}    
    if(other.pg->next_pgno  != pg->next_pgno    ){res++;printf("compare failed for  pg->next_pgno        \n");}    
    if(other.pg->entries    != pg->entries      ){res++;printf("compare failed for  pg->entries          \n");}    
    if(other.pg->hf_offset  != pg->hf_offset    ){res++;printf("compare failed for  pg->hf_offset        \n");}    
    if(other.pg->level      != pg->level        ){res++;printf("compare failed for  pg->level            \n");}    
    if(other.pg->type       != pg->type         ){res++;printf("compare failed for  pg->type             \n");}    
    int i;
    // double check header
    if(memcmp((void *)pg,(void *)other.pg,sizeof(PAGE)-sizeof(db_indx_t)))
    {
	res++;
	printf("compare failed in some unknown place in header:\n");
	unsigned int i;
	for(i=0;i<sizeof(PAGE)-sizeof(db_indx_t);i++)
	{
	    printf("%3d: %3x %3x\n",i,((byte *)pg)[i],((byte *)other.pg)[i]);
	}
    }

    // pg->type != 5 && !=3 pages are not really compressed: just memcmp
    if(pg->type != 5 && pg->type != 3)
    {
	if(memcmp((void *)pg,(void *)other.pg,pgsz))
	{
	    printf("compare:PAGETYPE:!=5 and memcmp failed\n");
	    res++;
	    printf("compare failed\n");
	}
	return(res);
    }

    // compare each key/data pair
    for(i=0;i<(type==5 ?  pg->entries/2 : pg->entries);i++)
    {
	if(pg->type==5)
	{
	    // compare keys
	    if(key(i)->len !=other.key(i)->len )
	    {
		printf("compare:key(%2d) len :  %2d != %2d\n",i,key(i)->len ,other.key(i)->len );
		res++;
	    }
	    if(key(i)->type!=other.key(i)->type)
	    {
		printf("compare:key(%2d) type:  %2d != %2d\n",i,key(i)->type,other.key(i)->type);
		res++;
	    }
	    if(memcmp(key(i)->data,other.key(i)->data,key(i)->len))
	    {
		printf("compare :key(%2d)\n",i);
		for(int k=0;k<key(i)->len;k++)
		{
		    int c=key(i)->data[k];
		    if(isalnum(c)){printf(" %c ",c);}
		    else{printf("%02x ",c);}
		}
		printf("\n");
		for(int k=0;k<key(i)->len;k++)
		{
		    int c=other.key(i)->data[k];
		    if(isalnum(c)){printf(" %c ",c);}
		    else{printf("%02x ",c);}
		}
		printf("\n");
		res++;printf("compare:key failed\n");
	    }
	    // compare data
	    if(data(i)->len !=other.data(i)->len )
	    {
		printf("compare:data(%2d) len :  %2d != %2d\n",i,data(i)->len ,other.data(i)->len );
		res++;
	    }
	    if(data(i)->type!=other.data(i)->type)
	    {
		printf("compare:data(%2d) type:  %2d != %2d\n",i,data(i)->type,other.key(i)->type);
		res++;
	    }
	    if(memcmp(data(i)->data,other.data(i)->data,data(i)->len))
	    {
		printf("compare :data(%2d)\n",i);
		for(int k=0;k<data(i)->len;k++)
		{
		    printf("%02x ",data(i)->data[k]);
		}
		printf("\n");
		for(int k=0;k<data(i)->len;k++)
		{
		    printf("%02x ",other.data(i)->data[k]);
		}
		printf("\n");
		res++;printf("compare:data failed\n");
	    }
	}
	else
	{
	    if(type!=3){errr("WordDBPage::Compare: unsupported type!=3");}
	    if(btikey(i)->len   != other.btikey(i)->len  ||
	       btikey(i)->type  != other.btikey(i)->type ||
	       btikey(i)->pgno  != other.btikey(i)->pgno ||
	       btikey(i)->nrecs != other.btikey(i)->nrecs   )
	    {
		printf("compare:btikey(%2d) failed\n",i);
		printf("this :len   :%4d type  :%4d pgno  :%4d nrecs :%4d \n",btikey(i)->len,btikey(i)->type,
		       btikey(i)->pgno,btikey(i)->nrecs);
		printf("other:len   :%4d type  :%4d pgno  :%4d nrecs :%4d \n",other.btikey(i)->len,other.btikey(i)->type,
		       other.btikey(i)->pgno,other.btikey(i)->nrecs);
		res++;

	    }
	    if(memcmp(btikey(i)->data,other.btikey(i)->data,btikey(i)->len))
	    {
		printf("compare :btikey(%2d)\n",i);
		for(int k=0;k<btikey(i)->len;k++)
		{
		    printf("%02x ",btikey(i)->data[k]);
		}
		printf("\n");
		for(int k=0;k<btikey(i)->len;k++)
		{
		    printf("%02x ",other.btikey(i)->data[k]);
		}
		printf("\n");
		res++;printf("compare:btikey failed\n");

	    }	    
	}
    }
    if(pg->entries>0)
    {
	int smallestoffset=min_v(pg->inp,pg->entries);
	int other_smallestoffset=min_v(other.pg->inp,other.pg->entries);
	if(smallestoffset!=other_smallestoffset)
	{
	    printf("compare fail:smallestoffset:%d other_smallestoffset:%d\n",smallestoffset,other_smallestoffset);
	    res++;
	}
    }

    return(res);
}




// Bit stream description
// | field[last] changed only | yes -> delta field[last]
// 


int show_page_ct=0;
// redo=0 -> 
// redo=1 -> oops, dont show!
// redo=2 -> 
void
WordDBPage::show(int redo)
{
  int i,j,dd,l;
//    if(redo==1){silent=1;}

  printf("************************************\n");
  printf("************************************\n");
  printf("************************************\n");
  printf("page size:%d\n",(int)pgsz);
  printf(" 00-07: Log sequence number.  file  : %d\n",                            pg->lsn.file            );      
  printf(" 00-07: Log sequence number.  offset: %d\n",                            pg->lsn.offset            );      
  printf(" 08-11: Current page number.  : %d\n",		               pg->pgno            );     
  printf(" 12-15: Previous page number. : %d\n",		               pg->prev_pgno         );
  printf(" 16-19: Next page number.     : %d\n",			       pg->next_pgno           );
  printf(" 20-21: Number of item pairs on the page. : %d\n",	               pg->entries           );  
  printf(" 22-23: High free byte page offset.       : %d\n",	               pg->hf_offset        );
  printf("    24: Btree tree level.                 : %d\n",                pg->level             );	
  printf("    25: Page type.                        : %d\n",                pg->type               );		


  printf("entry offsets:");
  for(i=0;i<pg->entries;i++){printf("%4d ",pg->inp[i]);}
  printf("\n");

  if(pg->type ==5)
  {

      WordRecord dud;
      WordKey prev;
      int pagecl=0;
      for(i=0;i<pg->entries;i++)
      {
	  if( (i%2) && dud.type==WORD_RECORD_NONE){continue;}
	  printf("\n||%c:%3d:off:%03d:invoff:%4d:len:%2d:typ:%x:",i%2 ? 'D' : 'K',i,e_offset(i),pgsz-e_offset(i),entry(i)->len,entry(i)->type);
	  if(i>0)
	  {
	      l=entry(i)->len+3;
	      dd=(int)(e_offset(i-1))-l;
	      dd-=dd%4;
	      printf("% 5d:: ",(e_offset(i)-dd));
	  }
	  if(!(i%2))
	  {
	      WordDBKey key(entry(i));
	      int fieldchanged[10];
	      char *wordchange=NULL;
	      printf("\"");
	      printf("%s",(char *)key.GetWord());
	      printf("\"");
	      for(j=0;j<20-key.GetWord().length();j++){printf(" ");}
	      printf("|");
	      for(j=1;j<word_key_info.nfields;j++){printf("%4x ",key.GetInSortOrder(j));}
	      printf("|");
	  
	      for(j=1;j<word_key_info.nfields;j++)
	      {
		  int diff=key.GetInSortOrder(j)-prev.GetInSortOrder(j);
		  if(diff<0){diff=key.GetInSortOrder(j);}
		  printf("%6d ",diff);
		  fieldchanged[j]=diff;
	      }

	      String &word=key.GetWord();
	      String &pword=prev.GetWord();
	      if(word==pword){printf("  00   ===");fieldchanged[0]=0;}
	      else
	      {
		  int fd=first_diff(word,pword);
		  fieldchanged[0]=fd+1;
		  wordchange=((char *)word)+fd;
		  printf("  %2d %s",fd,((char *)word)+fd);
	      }

	      int keycl=word_key_info.nfields;
	      for(j=1;j<word_key_info.nfields;j++)
	      {
		  if(fieldchanged[j]){keycl+=word_key_info.sort[j].bits;}
	      }
	      if(fieldchanged[0]){keycl+=3;keycl+=8*strlen(wordchange);}
	      printf("  ::%2d  %f",keycl,keycl/8.0);
	      pagecl+=keycl;
	      prev=key;
	  }
	  else
	  {
	      if(entry(i)->len>100){printf("WordDBPage::show: aaargh strange failing\n");return;}
	      for(j=0;j<entry(i)->len;j++)
	      {
		  printf("%02x ",entry(i)->data[j]);
	      }
	  }
      }
      printf("\n");
  }
  else
  if(1)
  {
      int n=0;
      // dump hex
      for(i=0;;i++)
      {
	  printf("%5d: ",n);
	  for(j=0;j<20;j++)
	  {
	      printf("%2x ",((byte *)pg)[n++]);
	      if(n>=pgsz){break;}
	  }
	  printf("\n");
	  if(n>=pgsz){break;}
      }
  }
  if(pg->type == 3)
  {
      for(i=0;i<pg->entries;i++)
      {
	  BINTERNAL *bie=GET_BINTERNAL(pg,i);
	  printf("%3d: off:%4d:len:%3d :type:%3d :pgno:%4d: nrecs:%4d:: ",i,pg->inp[i],bie->len,bie->type,bie->pgno,bie->nrecs);
	  WordDBKey key(bie);
	  for(j=0;j<bie->len-key.GetWord().length();j++){printf("%2x ",bie->data[j]);}
	  printf(" : ");
	  for(j=1;j<word_key_info.nfields;j++){printf("%5d ",key.GetInSortOrder(j));}
	  printf("\"%s\"\n",(char *)key.GetWord());
      }
  }

}
