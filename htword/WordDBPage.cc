//
// WordDBPage.cc
//
// WordDBPage: Implements specific compression scheme for
//                 Berkeley DB pages containing WordReferences objects.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDBPage.cc,v 1.1.2.2 2000/01/10 16:19:13 loic Exp $
//

#include"WordDBPage.h"
#include"WordDBCompress.h"
#include<ctype.h>

#define NBITS_CMPRTYPE 2
#define CMPRTYPE_NORMALCOMRPESS 0
#define CMPRTYPE_BADCOMPRESS 1

extern int word_debug_cmprcount;


// ***********************************************
// **********  Compression Versions **************
// ***********************************************

// never change NBITS_COMPRESS_VERSION ! (otherwise version tracking will fail)
#define NBITS_COMPRESS_VERSION 10

// IMPORTANT: change these EVERY time you change something that affects the compression
#define COMPRESS_VERSION 4
static const char *version_label[]={"INVALID_VERSION_0","INVALID_VERSION_1","INVALID_VERSION_2","14 Dec 1999","3 Jan 2000",NULL};

// returns the label of compression version v
static const char *
get_version_label(int v)
{
    // check if version number is ok
    if(COMPRESS_VERSION <0 || COMPRESS_VERSION>((sizeof(version_label)/sizeof(*version_label))-1))
    {
	errr("get_version_label: version_label[COMPRESS_VERSION] is not valid, please update version_label");
    }
    if( v >= (int)((sizeof(version_label)/sizeof(*version_label))-1) )
    {
	return("INVALID_VERSION");
    }
    // return label
    return(version_label[v]);
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
    // start by compressing this page
    Compressor *res=Compress(compress_debug);

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
static int first_diff(const String &s1,const String &s2)
{
    int j;
    for(j=0;j<s1.length() && j<s2.length() && s1[j]==s2[j];j++);
    return(j);
}

// ******* Uncompress Compressor into this page
int 
WordDBPage::Uncompress(Compressor *pin,int  ndebug, DB_CMPR_INFO *cmprInfo/*=NULL*/)
{
    debug=ndebug;
    if(debug>1){verbose=1;}
    if(verbose){printf("uuuuuuuuu WordDBPage::Uncompress: BEGIN\n");}
    

    // ** first check if versions are OK
    int read_version = pin->get_uint(NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
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


    // ** now see if this page was a normal or uncorrectly compressed page
    int cmprtype=pin->get_uint(NBITS_CMPRTYPE,"CMPRTYPE");   
    // two possible cases
    switch(cmprtype)
    {
    case CMPRTYPE_NORMALCOMRPESS:// this was a normaly compressed page
	Uncompress_main(pin);
	break;
    case CMPRTYPE_BADCOMPRESS:// this page did not compress correctly
	pin->get_zone((byte *)pg,pgsz*8,"INITIALBUFFER");
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
    // number arrays used to reconstruct the original page
    unsigned int **rnums=new unsigned int *[nnums];
    CHECK_MEM(rnums);
    // sizes of each array 
    int *rnum_sizes=new int[nnums];
    CHECK_MEM(rnum_sizes);
    // char differences between words
    byte *rworddiffs=NULL;
    int nrworddiffs;

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
	if(type==P_LBTREE){uncompress_data(in,0,key0.RecType());}
	nkeysleft--;
    }
    if(nkeysleft>0 && type==P_IBTREE){uncompress_key(in,1);nkeysleft--;}

    if(nkeysleft>0)
    {
	// ********* read numerical fields
	Uncompress_vals_chaged_flags(in,&(rnums[0]),&(rnum_sizes[0]));
	for(j=1;j<nnums;j++)
	{
	    if(verbose)printf("field %2d : start position:%4d  \n",j,in.size());
	    if(j==3 && verbose){in.verbose=2;}
	    rnum_sizes[j]=in.get_vals(&(rnums[j]),label_str("NumField",j));// ***
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
void 
WordDBPage::Uncompress_vals_chaged_flags(Compressor &in,unsigned int **pcflags,int *pn)
{
    int n=in.get_uint_vl(NBITS_NVALS,"FlagsField");
    unsigned int *cflags=new unsigned int[n];
    unsigned int ex=0;
    int nbits=num_bits(n);
    for(int i=0;i<n;i++)
    {
	ex=in.get_uint(WordKey::NFields(),label_str("cflags",i));
	cflags[i]=ex;
	int rep=in.get("rep");
	if(rep)
	{
	    rep=in.get_uint_vl(nbits,NULL);
	    for(int k=1;k<=rep;k++){cflags[k+i]=ex;}
	    i+=rep;
	}
    }

    *pn=n;
    *pcflags=cflags;
}
int 
WordDBPage::Uncompress_header(Compressor &in)
{
    pg->lsn.file     =in.get_uint_vl(  8*sizeof(pg->lsn.file    ),"page:lsn.file");
    pg->lsn.offset   =in.get_uint_vl(  8*sizeof(pg->lsn.offset  ),"page:lsn.offset");
    pg->pgno         =in.get_uint_vl(  8*sizeof(pg->pgno        ),"page:pgno");
    pg->prev_pgno    =in.get_uint_vl(  8*sizeof(pg->prev_pgno   ),"page:prev_pgno");
    pg->next_pgno    =in.get_uint_vl(  8*sizeof(pg->next_pgno   ),"page:next_pgno");
    pg->entries      =in.get_uint_vl(  8*sizeof(pg->entries     ),"page:entries");
    pg->hf_offset    =in.get_uint_vl(  8*sizeof(pg->hf_offset   ),"page:hf_offset");
    pg->level        =in.get_uint_vl(  8*sizeof(pg->level       ),"page:level");
    pg->type         =in.get_uint_vl(  8*sizeof(pg->type        ),"page:type");

    init();

    if(verbose)
    {
	printf("************************************\n");
	printf("********   WordDBPage::Uncompress: page header ***\n");
	printf("************************************\n");
	printf("page size:%d\n",(int)pgsz);
	printf(" 00-07: Log sequence number.  file  : %d\n",           pg->lsn.file   );      
	printf(" 00-07: Log sequence number.  offset: %d\n",           pg->lsn.offset );      
	printf(" 08-11: Current page number.  : %d\n",		       pg->pgno       );     
	printf(" 12-15: Previous page number. : %d\n",		       pg->prev_pgno  );
	printf(" 16-19: Next page number.     : %d\n",		       pg->next_pgno  );
	printf(" 20-21: Number of item pairs on the page. : %d\n",     pg->entries    );  
	printf(" 22-23: High free byte page offset.       : %d\n",     pg->hf_offset  );
	printf("    24: Btree tree level.                 : %d\n",     pg->level      );	
	printf("    25: Page type.                        : %d\n",     pg->type       );		
    }
    return OK;
}
void 
WordDBPage::Uncompress_rebuild(Compressor &in,unsigned int **rnums,int *rnum_sizes,int nnums0,byte *rworddiffs,int nrworddiffs)
{
    int irwordiffs=0;
    int nfields=WordKey::NFields();
    int *rnum_pos=new int[   nnums0];// current index count
    CHECK_MEM(rnum_pos);

    int ii,j;
    for(j=0;j<nnums0;j++){rnum_pos[j]=0;}

    int i0=0;
    if(type==P_IBTREE){i0=1;}// internal pages have particular first key

    WordDBKey pkey;
    WordDBKey akey=get_WordDBKey(i0);

    // reconstruct each key using previous key and  coded differences 
    for(ii=i0;ii<nk;ii++)
    {
	WordDBRecord arec;
	BINTERNAL bti;

	if(type==P_LBTREE)
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
	    // **** reconstruct the  word
	    if(flags&pow2(nfields-1))// check flags to see if word has changed
	    {
		foundfchange=1;
		if(rnum_pos[CNWORDDIFFLEN]>=rnum_sizes[CNWORDDIFFLEN]){errr("WordDBPage::Uncompress read wrong num worddiffs");}
		// get position of first character that changes in this word
		int diffpos=rnums[CNWORDDIFFPOS][rnum_pos[CNWORDDIFFPOS]++];
		// get size of changed part of the word
		int difflen=rnums[CNWORDDIFFLEN][rnum_pos[CNWORDDIFFLEN]++];
		int wordlen=diffpos+difflen;
		char *str=new char [wordlen+1];
		CHECK_MEM(str);
		// copy the unchanged part into str from previos key's word
		if(diffpos)strncpy(str,(char *)pkey.GetWord(),diffpos);
		// copy the changed part from coded word differences
		strncpy(str+diffpos,(char *)rworddiffs+irwordiffs,difflen);
		str[wordlen]=0;
		if(verbose)printf("key %3d word:\"%s\"\n",ii,str);
		akey.SetWord(str);
		irwordiffs+=difflen;
		delete [] str;

	    }else{akey.SetWord(pkey.GetWord());}
	    // **** reconstruct the numerical key fields
	    for(j=1;j<nfields;j++)
	    {
		// check flags to see if this field has changed
		int changed=flags&pow2(j-1);
		if(changed)
		{
		    // this field's number 
		    int k=CNFIELDS+j-1;
		    // current position within coded differences of this field
		    int indx=rnum_pos[k];
		    if(indx>=rnum_sizes[k]){errr("WordDBPage::Uncompress read wrong num of changes in a field");}
		    if(!foundfchange)
		    {
			// this is the first field that changes in this key
			// so difference is coded compared to value in pevious key
			akey.Set(j,rnums[k][indx]+pkey.Get(j));
		    }
		    else
		    {
			// this is NOT the first field that changes in this key
			// so difference is coded from 0
			akey.Set(j,rnums[k][indx]);
		    }
                    // we read 1 element from coded differences in this field
		    rnum_pos[k]++;
		    foundfchange=1;
		}
		else
		{
		    // no changes found, just copy from previous key
		    if(!foundfchange){akey.Set(j,pkey.Get(j));}
		    else{akey.Set(j,0);}
		}
	    }
	}
	// now insert key/data into page
	if(type==P_LBTREE)
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

// display
void 
WordDBPage::Uncompress_show_rebuild(unsigned int **rnums,int *rnum_sizes,int nnums0,byte *rworddiffs,int nrworddiffs)
{
    int i,j;
    if(verbose)
    {
	printf("WordDBPage::Uncompress_show_rebuild: rebuilt numerical fields\n");
	for(j=0;j<nnums0;j++)
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
WordDBPage::Compress(int ndebug, DB_CMPR_INFO *cmprInfo/*=NULL*/)
{
    debug=ndebug;
    if(debug>1){verbose=1;}

    Compressor *res=(Compressor *)new Compressor((cmprInfo ? 
						  pgsz/(1<<(cmprInfo->coefficient)) :
						  pgsz/4));
    CHECK_MEM(res);
    if(debug>0){res->set_use_tags();}

    res->put_uint(COMPRESS_VERSION,NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
    res->put_uint(CMPRTYPE_NORMALCOMRPESS,NBITS_CMPRTYPE,"CMPRTYPE");

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

	res->put_uint(COMPRESS_VERSION,NBITS_COMPRESS_VERSION,"COMPRESS_VERSION");
	res->put_uint(CMPRTYPE_BADCOMPRESS,NBITS_CMPRTYPE,"CMPRTYPE");

	res->put_zone((byte *)pg,pgsz*8,"INITIALBUFFER");
    }

    if(verbose)
    {
	printf("WordDBPage::Compress: Final bitstream result for %d\n",word_debug_cmprcount);
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
//        if(pg->type==P_IBTREE){show();}


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
//      for(j=1;j<nfields;j++)  {cnsizes[j]=word_key_info->sort[j].bits;}
//      cnsizes[CNFLAGS]=4;
//      cnsizes[CNWORDDIFFPOS ]=8;
//      cnsizes[CNWORDDIFFLEN ]=8;
    HtVector_byte worddiffs;
    

//bmt_START;
    // *************** extract values and wordiffs **************
    if(nk>0)
    {
	Compress_extract_vals_wordiffs(nums,nums_pos,nnums,worddiffs);
	if(verbose)Compress_show_extracted(nums,nums_pos,nnums,worddiffs);
    }

    // *************** init compression **************

//bmt_END;bmt_START;
    Compress_header(out);

    // *************** compress  values and wordiffs **************

    // compress first key(s)
    int nkeysleft=nk;
    if(nkeysleft>0)
    {
	compress_key(out,0);
	if(type==P_LBTREE){compress_data(out,0);}
	nkeysleft--;
    }
    if(nkeysleft>0 && type==P_IBTREE){compress_key(out,1);nkeysleft--;}

    if(nkeysleft>0)
    {
//bmt_END;bmt_START;
	// compress values
	Compress_vals(out,nums,nums_pos,nnums);
//bmt_END;bmt_START;

	// compress worddiffs
	int size=out.put_fixedbitl(worddiffs.begin(),worddiffs.size(),"WordDiffs");
	if(verbose)printf("compressed wordiffs : %3d values: %4d bits %4f bytes\n",worddiffs.size(),size,size/8.0);
//bmt_END;
    }

    // *************** cleanup **************

    delete [] nums ;
    delete [] nums_pos;

    return OK;
}

void 
WordDBPage::Compress_extract_vals_wordiffs(int *nums,int *nums_pos,int nnums0,HtVector_byte &worddiffs)
{
    WordDBKey pkey;

    int ii,j;
    int i0=0;
    if(type==P_IBTREE){i0=1;}// internal pages have particular first key
    for(ii=i0;ii<nk;ii++)
    {
	WordDBKey akey=get_WordDBKey(ii);

	if(type==P_LBTREE)
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
	    if(arec.type==WORD_RECORD_DATA)
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
	    for(j=1;j<akey.NFields();j++)
	    {
		int diff=akey.Get(j)-(foundfchange ? 0 : pkey.Get(j));
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
		nums[iflag]|=pow2(akey.NFields()-1);
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
WordDBPage::Compress_vals_changed_flags(Compressor &out,unsigned int *cflags,int n)
{
    int size=out.size();
    out.put_uint_vl(n,NBITS_NVALS,"FlagsField");
    unsigned int ex=0;
    int nbits=num_bits(n);
    for(int i=0;i<n;i++)
    {
	ex=cflags[i];
	out.put_uint(ex,WordKey::NFields(),label_str("cflags",i));
	int k;
	for(k=1;k+i<n;k++){if(ex!=cflags[i+k]){break;}}
	k--;
	if(k>0)
	{
	    out.put(1,"rep");
	    out.put_uint_vl(k,nbits,NULL);
	    i+=k;
	}
	else
	{out.put(0,"rep");}
    }
    size=out.size()-size;
    if(verbose)printf("compressed flags %2d : %3d values: %4d bits %8f bytes  : ended bit field pos:%6d\n",0,n,size,size/8.0,out.size());

}

void 
WordDBPage::Compress_vals(Compressor &out,int *nums,int *nums_pos,int nnums0)
{
    // the changed flags fields are particular
    Compress_vals_changed_flags(out,(unsigned int *)(nums+0*nk),nums_pos[0]);

    
    // compress the difference numbers for the numerical fields
    for( int j=1;j<nnums0;j++)
    {
	int nv=nums_pos[j];
	unsigned int *v=(unsigned int *)(nums+j*nk);
	if(j==3 && verbose){out.verbose=2;}
	int size=out.put_vals(v,nv,label_str("NumField",j));
	if(j==3 && verbose){out.verbose=0;}
	if(verbose)printf("compressed field %2d : %3d values: %4d bits %8f bytes  : ended bit field pos:%6d\n",j,n,size,size/8.0,out.size());
    }
}

void
WordDBPage::Compress_header(Compressor &out)
{
// no smart compression ... for now
    out.put_uint_vl(pg->lsn.file     ,  8*sizeof(pg->lsn.file    ),"page:lsn.file");
    out.put_uint_vl(pg->lsn.offset   ,  8*sizeof(pg->lsn.offset  ),"page:lsn.offset");
    out.put_uint_vl(pg->pgno         ,  8*sizeof(pg->pgno        ),"page:pgno");
    out.put_uint_vl(pg->prev_pgno    ,  8*sizeof(pg->prev_pgno   ),"page:prev_pgno");
    out.put_uint_vl(pg->next_pgno    ,  8*sizeof(pg->next_pgno   ),"page:next_pgno");
    out.put_uint_vl(pg->entries      ,  8*sizeof(pg->entries     ),"page:entries");
    out.put_uint_vl(pg->hf_offset    ,  8*sizeof(pg->hf_offset   ),"page:hf_offset");
    out.put_uint_vl(pg->level        ,  8*sizeof(pg->level       ),"page:level");
    out.put_uint_vl(pg->type         ,  8*sizeof(pg->type        ),"page:type");
}

void 
WordDBPage::Compress_show_extracted(int *nums,int *nums_pos,int nnums0,HtVector_byte &worddiffs)
{
    int i,j;
    int *cnindexe2=new int[   nnums0];
    CHECK_MEM(cnindexe2);
    for(j=0;j<nnums0;j++){cnindexe2[j]=0;}
    int w=0;
    int mx=(nk>worddiffs.size() ? nk : worddiffs.size());
    for(i=0;i<mx;i++)
    {
	printf("%3d: ",i);
	for(j=0;j<nnums0;j++)
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
    if(other.pgsz           != pgsz           ){res++;printf("compare failed for  pgsz                 \n");}
    if(other.pg->lsn.file   != pg->lsn.file   ){res++;printf("compare failed for  pg->lsn.file         \n");}
    if(other.pg->lsn.offset != pg->lsn.offset ){res++;printf("compare failed for  pg->lsn.offset       \n");}
    if(other.pg->pgno       != pg->pgno       ){res++;printf("compare failed for  pg->pgno             \n");}
    if(other.pg->prev_pgno  != pg->prev_pgno  ){res++;printf("compare failed for  pg->prev_pgno        \n");}
    if(other.pg->next_pgno  != pg->next_pgno  ){res++;printf("compare failed for  pg->next_pgno        \n");}
    if(other.pg->entries    != pg->entries    ){res++;printf("compare failed for  pg->entries          \n");}
    if(other.pg->hf_offset  != pg->hf_offset  ){res++;printf("compare failed for  pg->hf_offset        \n");}
    if(other.pg->level      != pg->level      ){res++;printf("compare failed for  pg->level            \n");}
    if(other.pg->type       != pg->type       ){res++;printf("compare failed for  pg->type             \n");}
    int i,k;
    // double check header
    if(memcmp((void *)pg,(void *)other.pg,sizeof(PAGE)-sizeof(db_indx_t)))
    {
	res++;
	printf("compare failed in some unknown place in header:\n");
	for(i=0;i<(int)(sizeof(PAGE)-sizeof(db_indx_t));i++)
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
    for(i=0;i<(type==P_LBTREE ?  pg->entries/2 : pg->entries);i++)
    {
	if(pg->type==P_LBTREE)
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
		for(k=0;k<key(i)->len;k++)
		{
		    int c=key(i)->data[k];
		    if(isalnum(c)){printf(" %c ",c);}
		    else{printf("%02x ",c);}
		}
		printf("\n");
		for(k=0;k<key(i)->len;k++)
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
		for(k=0;k<data(i)->len;k++)
		{
		    printf("%02x ",data(i)->data[k]);
		}
		printf("\n");
		for(k=0;k<data(i)->len;k++)
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
		for(k=0;k<btikey(i)->len;k++)
		{
		    printf("%02x ",btikey(i)->data[k]);
		}
		printf("\n");
		for(k=0;k<btikey(i)->len;k++)
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
	int smallestoffset=HtMaxMin::min_v(pg->inp,pg->entries);
	int other_smallestoffset=HtMaxMin::min_v(other.pg->inp,other.pg->entries);
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
	      WordDBKey tkey(entry(i));
	      int fieldchanged[10];
	      char *wordchange=NULL;
	      printf("\"");
	      printf("%s",(char *)tkey.GetWord());
	      printf("\"");
	      for(j=0;j<20-tkey.GetWord().length();j++){printf(" ");}
	      printf("|");
	      for(j=1;j<tkey.NFields();j++){printf("%4x ",tkey.Get(j));}
	      printf("|");
	  
	      for(j=1;j<tkey.NFields();j++)
	      {
		  int diff=tkey.Get(j)-prev.Get(j);
		  if(diff<0){diff=tkey.Get(j);}
		  printf("%6d ",diff);
		  fieldchanged[j]=diff;
	      }

	      String &word=tkey.GetWord();
	      String &pword=prev.GetWord();
	      if(word==pword){printf("  00   ===");fieldchanged[0]=0;}
	      else
	      {
		  int fd=first_diff(word,pword);
		  fieldchanged[0]=fd+1;
		  wordchange=((char *)word)+fd;
		  printf("  %2d %s",fd,((char *)word)+fd);
	      }

	      int keycl=tkey.NFields();
	      for(j=1;j<tkey.NFields();j++)
	      {
		  if(fieldchanged[j]){keycl+=WordKeyInfo::Get()->sort[j].bits;}
	      }
	      if(fieldchanged[0]){keycl+=3;keycl+=8*strlen(wordchange);}
	      printf("  ::%2d  %f",keycl,keycl/8.0);
	      pagecl+=keycl;
	      prev=tkey;
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
      int nn=0;
      // dump hex
      for(i=0;;i++)
      {
	  printf("%5d: ",nn);
	  for(j=0;j<20;j++)
	  {
	      printf("%2x ",((byte *)pg)[nn++]);
	      if(nn>=pgsz){break;}
	  }
	  printf("\n");
	  if(nn>=pgsz){break;}
      }
  }
  if(pg->type == 3)
  {
      for(i=0;i<pg->entries;i++)
      {
	  BINTERNAL *bie=GET_BINTERNAL(pg,i);
	  printf("%3d: off:%4d:len:%3d :type:%3d :pgno:%4d: nrecs:%4d:: ",i,pg->inp[i],bie->len,bie->type,bie->pgno,bie->nrecs);
	  WordDBKey tkey(bie);
	  for(j=0;j<bie->len-tkey.GetWord().length();j++){printf("%2x ",bie->data[j]);}
	  printf(" : ");
	  for(j=1;j<tkey.NFields();j++){printf("%5d ",tkey.Get(j));}
	  printf("\"%s\"\n",(char *)tkey.GetWord());
      }
  }

}


