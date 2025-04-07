//
// WordDBPage.h
//
// WordDBPage: Implements specific compression scheme for
//                 Berkeley DB pages containing WordReferences objects.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordDBPage.h,v 1.8 2004/05/28 13:15:26 lha Exp $
//
//
// Access to Berkeley DB internal
//

#ifndef _WordDBPage_h_
#define _WordDBPage_h_

extern "C"
{
#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "db_page.h"
#include "common_ext.h"
}

#include "WordDBCompress.h"
#include "WordBitCompress.h"
#include "WordRecord.h"
#include "WordKey.h"


#define WORD_ALIGN_TO(v,a) ( (v)%(a) ? (v+((a)-(v)%(a))) : v)
#define NBITS_KEYLEN 16
#define NBITS_DATALEN 16

// ***********************************************
// *************** WordDBRecord  *****************
// ***********************************************

// WordRecord with added functionalities to help with compression/decompression
class WordDBRecord:public WordRecord
{
public:

  // retreive WordRecord data/stats from coded numbers
  void set_decompress (unsigned int **data, int *indexes, int i, int pdata,
                       int pstat0, int pstat1)
  {
    if (i >= indexes[pstat0])
    {                           // were pas the end of coded stats, so this can't be a stat
      type = DefaultType ();
      if (type == WORD_RECORD_DATA)
      {
        info.data = data[pdata][i - indexes[pstat0]];
      }
      else
      {
        info.data = 0;
      }
    }
    else
    {                           // this is a stat
      type = WORD_RECORD_STATS;
      info.stats.noccurrence = data[pstat0][i];
      info.stats.ndoc = data[pstat1][i];
    }
  }
WordDBRecord ():WordRecord ()
  {;
  }
  WordDBRecord (hlbyte * dat, int len, int rectyp):WordRecord ()
  {
    type = (rectyp ? DefaultType () : WORD_RECORD_STATS);
    Unpack (String ((char *) dat, len));
  }
  WordDBRecord (BKEYDATA * ndata, int rectyp):WordRecord ()
  {                             // typ: 0->stat 1->data
    type = (rectyp ? DefaultType () : WORD_RECORD_STATS);
    Unpack (String ((char *) ndata->data, ndata->len));
  }
};


// ***********************************************
// ****************  WordDBKey   *****************
// ***********************************************

// WordKey with added functionalities to help with compression/decompression
class WordDBKey:public WordKey
{
  BKEYDATA *key;
public:

  int RecType ()
  {
    return (GetWord ()[0] != 1 ? 1 : 0);
  }
  WordDBKey ():WordKey ()
  {
    key = NULL;
  }
WordDBKey (BKEYDATA * nkey):WordKey ()
  {
    key = nkey;
    Unpack (String ((char *) key->data, key->len));
  }
  int is_null ()
  {
    errr ("UNUSED");
    if (GetWord ().length () == 0)
    {
      for (int j = 1; j < NFields (); j++)
      {
        if (Get (j) != 0)
        {
          errr ("WordDBKey::is_null key has 0 len word but is not null");
        }
      }
      return 1;
    }
    return 0;
  }
WordDBKey (BINTERNAL * nkey):WordKey ()
  {
    key = NULL;
    if (nkey->len == 0)
    {
      ;                         //      errr("WordDBKey::WordDBKey(BINTERNAL) : nkey->len==0");
    }
    else
    {
      Unpack (String ((char *) nkey->data, nkey->len));
    }
  }
  WordDBKey (hlbyte * data, int len):WordKey ()
  {
    key = NULL;
    if (!data || !len)
    {
      errr ("WordDBKey::WordDBKey(data,len) !data || !len");
    }
    Unpack (String ((char *) data, len));
  }
};


// ***********************************************
// ****************  WordDBPage  *****************
// ***********************************************

// encapsulation of Berkeley DB BTREE page.
// this one knows how to compress/decompress itself
class WordDBPage
{
public:
  int n;                        // number of entries
  int nk;                       // number of keys
  int type;                     // for now 3(btreeinternal) && 5(leave:normal case) are allowed
  int pgsz;

  PAGE *pg;                     // pointer to BerkeleyDB BTREE page structure

  // assert this page is a leave
  void isleave ()
  {
    if (type != P_LBTREE)
    {
      errr ("WordDBPage::isleave: trying leave specific on non leave");
    }
  }

  // assert this page is an internal (non-leave) page
  void isintern ()
  {
    if (type != P_IBTREE)
    {
      errr
        ("WordDBPage::isintern: trying btreeinternal  specific on non btreeinternal page type");
    }

  }

  // get the i'th key stored in this page
  WordDBKey get_WordDBKey (int i)
  {
    if (type == P_LBTREE)
    {
      return (WordDBKey (key (i)));
    }
    else if (type == P_IBTREE)
    {
      return (WordDBKey (btikey (i)));
    }
    else
    {
      errr ("WordDBPage:get_WordDBKey: bad page type");
    }
    return WordDBKey ();
  }

  // ******************* Accessors to packed entries ****************

  // get the i'th key stored in this (internal==nonleave) page. (ptr to packed)
  BINTERNAL *btikey (int i)
  {
    if (i < 0 || i >= pg->entries)
    {
      printf ("btikey:%d\n", i);
      errr ("WordDBPage::btikey out iof bounds");
    }
    isintern ();
    return (GET_BINTERNAL (pg, i));
  }
  // get the i'th entry stored in this (nonleave) page. (ptr to packed)
  // an entry can either be a key or a data entry
  BKEYDATA *entry (int i)
  {
    if (i < 0 || i >= pg->entries)
    {
      printf ("entry:%d\n", i);
      errr ("WordDBPage::entry out iof bounds");
    }
    isleave ();
    return (GET_BKEYDATA (pg, i));
  }
  // get the i'th key stored in this (leave) page. (ptr to packed)
  BKEYDATA *key (int i)
  {
    if (i < 0 || 2 * i >= pg->entries)
    {
      printf ("key:%d\n", i);
      errr ("WordDBPage::key out iof bounds");
    }
    isleave ();
    return (GET_BKEYDATA (pg, i * 2));
  }
  // get the i'th data stored in this (leave) page. (ptr to packed)
  BKEYDATA *data (int i)
  {
    if (i < 0 || 2 * i + 1 >= pg->entries)
    {
      printf ("data:%d\n", i);
      errr ("WordDBPage::data out iof bounds");
    }
    isleave ();
    return (GET_BKEYDATA (pg, i * 2 + 1));
  }


  // ********************* Inserting entries into a page ***************

  int insert_pos;               // offset in page of last inserted entry 
  int insert_indx;              // index of next entry to be inserted

  int e_offset (int i)
  {
    return ((int) (pg->inp[i]));
  }

  // allocate space (in the db page) for adding an entry to this page
  void *alloc_entry (int size)
  {
    size = WORD_ALIGN_TO (size, 4);
    int inp_pos = ((hlbyte *) & (pg->inp[insert_indx])) - (hlbyte *) pg;
    insert_pos -= size;
    if (insert_pos <= inp_pos)
    {
      show ();
      printf
        ("alloc_entry: allocating size:%4d entrynum:insert_indx:%4d at:insert_pos:%4d\n",
         size, insert_indx, insert_pos);
      errr ("WordDBPage::alloc_entry: PAGE OVERFLOW");
    }
    pg->inp[insert_indx++] = insert_pos;
    return ((void *) ((hlbyte *) pg + insert_pos));
  }


  // add a data entry to this page
  void insert_data (WordDBRecord & wrec)
  {
    isleave ();
    if (!(insert_indx % 2))
    {
      errr ("WordDBPage::insert_data data must be an odd number!");
    }
    String prec;
    wrec.Pack (prec);
    int len = prec.length ();
    int size = len + (sizeof (BKEYDATA) - 1);

    BKEYDATA *dat = (BKEYDATA *) alloc_entry (size);
    dat->len = len;
    dat->type = 1;              //!!!!!!!!!!!!!
    memcpy ((void *) dat->data, (void *) (char *) prec, len);
  }
  // add a key entry to this page
  void insert_key (WordDBKey & ky)
  {
    isleave ();
    if (insert_indx % 2)
    {
      errr ("WordDBPage::insert_key key must be an even number!");
    }
    String pkey;
    ky.Pack (pkey);
    int keylen = pkey.length ();
    int size = keylen + (sizeof (BKEYDATA) - 1);
    BKEYDATA *bky = (BKEYDATA *) alloc_entry (size);
    bky->len = keylen;
    bky->type = 1;              // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    memcpy ((void *) bky->data, (void *) (char *) pkey, keylen);
  }
  // add a key entry to this internal page
  void insert_btikey (WordDBKey & ky, BINTERNAL & bti, int empty = 0)
  {
    isintern ();
    int keylen = 0;
    String pkey;
    if (!empty)
    {
      ky.Pack (pkey);
      keylen = pkey.length ();
    }
    int size = keylen + ((hlbyte *) & (bti.data)) - ((hlbyte *) & bti);     // pos of data field in BINTERNAL
    if (empty)
    {
      if (verbose)
      {
        printf
          ("WordDBPage::insert_btikey: empty : BINTERNAL:%d datapos:%d keylen:%d size:%d alligned to:%d\n",
           (int) sizeof (BINTERNAL),
           (int) (((hlbyte *) & (bti.data)) - ((hlbyte *) & bti)), keylen, size,
           WORD_ALIGN_TO (size, 4));
      }
    }

    BINTERNAL *btik = (BINTERNAL *) alloc_entry (size);
    btik->len = (empty ? 0 : keylen);
    btik->type = 1;             // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    btik->pgno = bti.pgno;
    btik->nrecs = bti.nrecs;
    if (!empty)
    {
      memcpy ((void *) btik->data, (void *) (char *) pkey, keylen);
    }
//    else
//    {btik->data[0]=0;}// just to avoid uninit memory read
  }
  int entry_struct_size ()
  {
    return (type == P_IBTREE ? sizeof (BINTERNAL) : sizeof (BKEYDATA)) - 1;
  }
  int entry_size (int i)
  {
    return entry_struct_size () + (type ==
                                   P_IBTREE ? btikey (i)->len : key (i)->len);
  }





  // ************** Comrpession/Uncompression ***************************

  // The compression functions
  void Compress_extract_vals_wordiffs (int *nums, int *nums_pos, int nnums,
                                       HtVector_byte & wordiffs);
  void Compress_show_extracted (int *nums, int *nums_pos, int nnums,
                                HtVector_byte & wordiffs);
  void Compress_vals (Compressor & out, int *nums, int *nums_pos, int nnums);
  void Compress_vals_changed_flags (Compressor & out, unsigned int *cflags,
                                    int n);
  void Compress_header (Compressor & out);
  int Compress_main (Compressor & out);
  Compressor *Compress (int debug = 0, DB_CMPR_INFO * cmprInfo = NULL);

  // The uncompression functions
  int Uncompress (Compressor * pin, int debug = 0, DB_CMPR_INFO * cmprInfo =
                  NULL);
  int Uncompress_main (Compressor * pin);
  void Uncompress_vals_chaged_flags (Compressor & in, unsigned int **pcflags,
                                     int *pn);
  int Uncompress_header (Compressor & in);
  void Uncompress_rebuild (unsigned int **rnums, int *rnum_sizes, int nnums,
                           hlbyte * rworddiffs, int nrworddiffs);
  void Uncompress_show_rebuild (unsigned int **rnums, int *rnum_sizes,
                                int nnums, hlbyte * rworddiffs,
                                int nrworddiffs);

  int TestCompress (int debuglevel);
  int Compare (WordDBPage & other);

  // the following functions are use to compress/uncompress
  // keys/data directly
  // This is necesary for the first key/data elements of the page
  void compress_key (Compressor & out, int i)
  {
    if (type == P_IBTREE)
    {
      int len = btikey (i)->len;
      out.put_uint (len, NBITS_KEYLEN, label_str ("seperatekey_len", i));
      if (verbose)
      {
        printf
          ("WordDBPage::compress_key:compress(typ3):%d ::: sizeof(BINTERNAL):%d\n",
           len, (int) sizeof (BINTERNAL));
      }
      out.put_uint (btikey (i)->len, sizeof (btikey (i)->len) * 8,
                    label_str ("seperatekey_bti_len", i));
      out.put_uint (btikey (i)->type, sizeof (btikey (i)->type) * 8,
                    label_str ("seperatekey_bti_type", i));
      out.put_uint (btikey (i)->pgno, sizeof (btikey (i)->pgno) * 8,
                    label_str ("seperatekey_bti_pgno", i));
      out.put_uint (btikey (i)->nrecs, sizeof (btikey (i)->nrecs) * 8,
                    label_str ("seperatekey_bti_nrecs", i));
      if (len)
      {
        out.put_zone ((hlbyte *) btikey (i)->data, 8 * len,
                      label_str ("seperatekey_btidata", i));
      }
    }
    else
    {
      int len = key (i)->len;
      out.put_uint (len, NBITS_KEYLEN, label_str ("seperatekey_len", i));
      if (verbose)
      {
        printf ("WordDBPage::compress_key: compress(typ5):%d\n", len);
      }
      out.put_zone ((hlbyte *) key (i)->data, 8 * len,
                    label_str ("seperatekey_data", i));
    }
  }
  void compress_data (Compressor & out, int i)
  {
    int len = data (i)->len;
    out.put_uint (len, NBITS_DATALEN, label_str ("seperatedata_len", i));
    if (verbose)
    {
      printf ("WordDBPage::compress_data: compressdata(typ5):%d\n", len);
    }
    out.put_zone ((hlbyte *) data (i)->data, 8 * len,
                  label_str ("seperatedata_data", i));
  }
  WordDBKey uncompress_key (Compressor & in, int i)
  {
    WordDBKey res;
    int len = in.get_uint (NBITS_KEYLEN, label_str ("seperatekey_len", i));
    if (verbose)
    {
      printf ("WordDBPage::uncompress_key: seperatekey:len:%d\n", len);
    }

    if (type == P_IBTREE)
    {
      if (len == 0 && i != 0)
      {
        errr ("WordDBPage::uncompress_key: keylen=0 &&    i!=0");
      }
      BINTERNAL bti;
      bti.len =
        in.get_uint (sizeof (bti.len) * 8,
                     label_str ("seperatekey_bti_len", i));
      bti.type =
        in.get_uint (sizeof (bti.type) * 8,
                     label_str ("seperatekey_bti_type", i));
      bti.pgno =
        in.get_uint (sizeof (bti.pgno) * 8,
                     label_str ("seperatekey_bti_pgno", i));
      bti.nrecs =
        in.get_uint (sizeof (bti.nrecs) * 8,
                     label_str ("seperatekey_bti_nrecs", i));
      if (len != bti.len)
      {
        errr ("WordDBPage::uncompress_key: incoherence: len!=bti.len");
      }
      if (len)
      {
        hlbyte *gotdata = new hlbyte[len];
        CHECK_MEM (gotdata);
        in.get_zone (gotdata, 8 * len, label_str ("seperatekey_btidata", i));
        res = WordDBKey (gotdata, len);
        delete[]gotdata;
      }
      insert_btikey (res, bti, (len == 0 ? 1 : 0));
    }
    else
    {
      hlbyte *gotdata = new hlbyte[len];
      CHECK_MEM (gotdata);
      in.get_zone (gotdata, 8 * len, label_str ("seperatekey_data", i));
      res = WordDBKey (gotdata, len);
      insert_key (res);
      delete[]gotdata;
    }
    return res;
  }
  WordDBRecord uncompress_data (Compressor & in, int i, int rectyp)
  {
    WordDBRecord res;
    int len = in.get_uint (NBITS_DATALEN, label_str ("seperatedata_len", i));
    if (verbose)
      printf ("uncompressdata:len:%d\n", len);
    hlbyte *gotdata = new hlbyte[len];
    CHECK_MEM (gotdata);
    in.get_zone (gotdata, 8 * len, label_str ("seperatedata_data", i));
    res = WordDBRecord (gotdata, len, rectyp);
    insert_data (res);
    delete[]gotdata;
    return res;
  }


  // exctracted numerical fields

  const char *number_field_label (int j)
  {
    if (j > 0 && j < WordKey::NFields ())
    {
      return (char *) (WordKey::Info ()->sort[j].name);
    }
    if (j == CNFLAGS)
      return "CNFLAGS      ";
    if (j == CNDATASTATS0)
      return "CNDATASTATS0 ";
    if (j == CNDATASTATS1)
      return "CNDATASTATS1 ";
    if (j == CNDATADATA)
      return "CNDATADATA   ";
    if (j == CNBTIPGNO)
      return "CNBTIPGNO    ";
    if (j == CNBTINRECS)
      return "CNBTINRECS   ";
    if (j == CNWORDDIFFPOS)
      return "CNWORDDIFFPOS";
    if (j == CNWORDDIFFLEN)
      return "CNWORDDIFFLEN";
    return "BADFIELD";
  }
  // positions of different fileds in 
  // number arrays that are extracted
  int CNFLAGS;                  // FLAGS: which key-fields have changed 
  int CNFIELDS;                 // first numerical field
  int CNDATASTATS0;             // word record - stats element 0
  int CNDATASTATS1;             // word record - stats element 1
  int CNDATADATA;               // word record - data
  int CNBTIPGNO;                // internal page: page pointed at by node
  int CNBTINRECS;               // internal page: ??
  int CNWORDDIFFPOS;            // position of first caracter that changed in word
  int CNWORDDIFFLEN;            // number of chars that  changed in word
  int nnums;


  // ************** DEBUGING/BENCHMARKING  ***************
  void show ();
  int verbose;
  int debug;


  // ************** Initialization/Destruction *****************

  // initialize when header is valid
  void init ()
  {
    type = pg->type;
    n = pg->entries;
    nk = (type == P_LBTREE ? n / 2 : n);
    insert_pos = pgsz;
    insert_indx = 0;
  }

  void init0 ()
  {
    CNFLAGS = 0;
    CNFIELDS = 1;
    CNDATASTATS0 = WordKey::NFields ();
    CNDATASTATS1 = WordKey::NFields () + 1;
    CNDATADATA = WordKey::NFields () + 2;
    CNBTIPGNO = WordKey::NFields () + 3;
    CNBTINRECS = WordKey::NFields () + 4;
    CNWORDDIFFPOS = WordKey::NFields () + 5;
    CNWORDDIFFLEN = WordKey::NFields () + 6;
    nnums = (CNWORDDIFFLEN + 1);

    pg = NULL;
    pgsz = 0;
    n = 0;
    nk = 0;
    type = -1;
    verbose = 0;
    debug = 0;
    insert_pos = pgsz;
    insert_indx = 0;
  }

  // db page was created here, destroy it
  void delete_page ()
  {
    if (!pg)
    {
      errr ("WordDBPage::delete_page: pg==NULL");
    }
    delete[]pg;
    pg = NULL;
  }
  // unlink db page from this encapsulation
  void unset_page ()
  {
    if (!pg)
    {
      errr ("WordDBPage::unset_page: pg==NULL");
    }
    pg = NULL;
  }
  // the DB page must be unset or deleted
  // before destroying this encapsulation
  ~WordDBPage ()
  {
    if (pg)
    {
      errr ("WordDBPage::~WordDBPage: page not empty");
    }
  }
  WordDBPage (int npgsz)
  {
    init0 ();
    pgsz = npgsz;
    pg = (PAGE *) (new hlbyte[pgsz]);
    CHECK_MEM (pg);
    insert_pos = pgsz;
    insert_indx = 0;
  }
  WordDBPage (const uint8_t * buff, int buff_length)
  {
    init0 ();
    pg = (PAGE *) buff;
    pgsz = buff_length;
    insert_pos = pgsz;
    insert_indx = 0;
    init ();
  }
};


#endif // _WordDBPage_h_
