//
// WordDBCompress.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDBCompress.cc,v 1.1.2.20 2000/09/14 03:13:27 ghutchis Exp $
//
/*

 BTREE original page layout in Berkeley DB

 *	+-----------------------------------+
 *	|    lsn    |   pgno    | prev pgno |
 *	+-----------------------------------+
 *	| next pgno |  entries  | hf offset |
 *	+-----------------------------------+
 *	|   level   |   type    |   index   |
 *	+-----------------------------------+
 *	|   index   | free -->              |
 *	+-----------+-----------------------+
 *	|   	 F R E E A R E A            |
 *	+-----------------------------------+
 *	|              <-- free |   item    |
 *	+-----------------------------------+
 *	|   item    |   item    |   item    |
 *	+-----------------------------------+
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <errno.h>

extern "C" {
#include "db_int.h"
#include "db_page.h"
}

#include "lib.h"
#include "WordDBCompress.h"
#include "WordBitCompress.h"
#include "WordKeyInfo.h"
#include "WordKey.h"
#include "WordRecord.h"
#include "WordDB.h"
#include "HtMaxMin.h"

/*
 *   WordDBCompress: C-callbacks, actually called by Berkeley-DB
 *      they just call their WordDBCompress equivalents (by using user_data)
 */
extern "C"
{

int WordDBCompress_compress_c(DB_ENV*, const u_int8_t* inbuff, int inbuff_length, u_int8_t** outbuffp, int* outbuff_lengthp, void *user_data)
{
    if(!user_data) {
      fprintf(stderr, "WordDBCompress_compress_c:: user_data is NULL");
      return NOTOK;
    }
    return ((WordDBCompress *)user_data)->Compress((unsigned char*)inbuff, inbuff_length, (unsigned char**)outbuffp, outbuff_lengthp);
}

int WordDBCompress_uncompress_c(DB_ENV*, const u_int8_t* inbuff, int inbuff_length, u_int8_t* outbuff, int outbuff_length, void *user_data)
{
    if(!user_data) {
      fprintf(stderr, "WordDBCompress_uncompress_c:: user_data is NULL");
      return NOTOK;
    }
    return ((WordDBCompress *)user_data)->Uncompress((unsigned char *)inbuff, inbuff_length, (unsigned char*)outbuff, outbuff_length);
}

}

//
// Symbolic names for the data in WordDBEncoded.values[..] arrays
//
//
// Each entry in a leaf/internal page has a flag that shows how
// the entry is coded.
//
#define WORD_CMPR_VAL_FLAGS	0
//
// Btree: Numerical fields of WordKey
//
#define WORD_CMPR_VAL_FIELDS	1
//
// Btree/Internal: BINTERNAL.pgno
//
#define WORD_CMPR_VAL_PGNO	(WORD_CMPR_VAL_FIELDS + WORD_KEY_MAX_NFIELDS)
//
// Btree/Leaf: Length of record string
//
#define WORD_CMPR_VAL_RLENGTH	WORD_CMPR_VAL_PGNO
//
// Btree/Internal: BINTERNAL.nrecs
//
#define WORD_CMPR_VAL_NRECS	(WORD_CMPR_VAL_PGNO + 1)
//
// Btree/Leaf: Value of integer if record contains a single integer value
//
#define WORD_CMPR_VAL_RVALUE    WORD_CMPR_VAL_NRECS
//
// Btree: length of WordKey.GetWord() prefix shared with previous entry
//
#define WORD_CMPR_VAL_PREFIX	(WORD_CMPR_VAL_PGNO + 2)
//
// Btree: length of WordKey.GetWord() suffix to add to previous key
// shared prefix.
//
#define WORD_CMPR_VAL_SUFFIX	(WORD_CMPR_VAL_PGNO + 3)
//
// Symbolic name for the last index 
//
#define WORD_CMPR_VAL_LAST	WORD_CMPR_VAL_SUFFIX

#define WORD_CMPR_VAL_ARRAY_SIZE (WORD_CMPR_VAL_LAST + 1)
//
// Number of bits needed to encode the maximum array size value
//
#define WORD_CMPR_VAL_ARRAY_SIZE_BITS 8

//
// Number of bits used in flags
//
#define WORD_CMPR_VAL_FLAGS_BITS (WORD_CMPR_VAL_LAST + 1)
//
// Values for the WORD_CMPR_VAL_FLAGS bit field
//
#define WORD_CMPR_VAL_FLAGS_FIELD(n)	(1 << (WORD_CMPR_VAL_FIELDS + (n)))
#define WORD_CMPR_VAL_FLAGS_PGNO	(1 << (WORD_CMPR_VAL_PGNO))
#define WORD_CMPR_VAL_FLAGS_RLENGTH	WORD_CMPR_VAL_FLAGS_PGNO
#define WORD_CMPR_VAL_FLAGS_NRECS	(1 << (WORD_CMPR_VAL_NRECS))
#define WORD_CMPR_VAL_FLAGS_RVALUE	WORD_CMPR_VAL_FLAGS_NRECS
#define WORD_CMPR_VAL_FLAGS_PREFIX	(1 << (WORD_CMPR_VAL_PREFIX))
#define WORD_CMPR_VAL_FLAGS_SUFFIX	(1 << (WORD_CMPR_VAL_SUFFIX))
#define WORD_CMPR_VAL_FLAGS_STRING	(1 << (WORD_CMPR_VAL_LAST + 1))
#define WORD_CMPR_VAL_FLAGS_EMPTY	(1 << (WORD_CMPR_VAL_LAST + 2))
#define WORD_CMPR_VAL_FLAGS_RECORD_EQ	(1 << (WORD_CMPR_VAL_LAST + 3))
#define WORD_CMPR_VAL_FLAGS_RECORD_NO	(1 << (WORD_CMPR_VAL_LAST + 4))
#define WORD_CMPR_VAL_FLAGS_RECORD_STR	(1 << (WORD_CMPR_VAL_LAST + 5))

//
// Storage for values extracted from a PAGE structure
//
class WordDBEncoded {
public:
  inline WordDBEncoded() {
    Init();
  }
  inline ~WordDBEncoded() {
    free(strings);
    for(int i = 0; i < WORD_CMPR_VAL_ARRAY_SIZE; i++)
      free(values[i]);
  }

  inline void Init() {
    strings_size = 32;
    strings = (unsigned char*)malloc(strings_size);

    for(int i = 0; i < WORD_CMPR_VAL_ARRAY_SIZE; i++) {
      values_size[i] = 32;
      values[i] = (unsigned int*)malloc(values_size[i] * sizeof(unsigned int));
    }

    Clear();
  }

  inline void Clear() {
    strings_length = 0;
    strings_idx = 0;

    for(int i = 0; i < WORD_CMPR_VAL_ARRAY_SIZE; i++) {
      values_length[i] = 0;
      values_idx[i] = 0;
    }    
  }

  inline void AllocateStrings(unsigned int size) {
    strings_size = size;
    strings = (unsigned char*)realloc(strings, strings_size);
  }

  inline void CheckStrings(int index) {
    while(index >= strings_size)
      AllocateStrings(strings_size * 2);
  }

  inline void AllocateValues(int what, unsigned int size) {
    values_size[what] = size;
    values[what] = (unsigned int*)realloc(values[what], values_size[what] * sizeof(unsigned int));
  }

  inline void CheckValues(int what, int index) {
    while(index >= values_size[what])
      AllocateValues(what, values_size[what] * 2);
  }

  inline void PushValue(int what, unsigned int value) {
    CheckValues(what, values_length[what]);
    values[what][values_length[what]] = value;
    values_length[what]++;
  }

  inline void PushString(const unsigned char* string, int length) {
    CheckStrings(strings_length + length);
    memcpy(strings + strings_length, string, length);
    strings_length += length;
  }

  inline unsigned int ShiftValue(int what) {
    if(values_idx[what] >= values_length[what]) {
      fprintf(stderr, "WordDBEncoded::ShiftValue: what = %d, (idx = %d) >= (length = %d)\n", what, values_idx[what], values_length[what]);
      abort();
    }
    return values[what][values_idx[what]++];
  }

  inline unsigned char* ShiftString(int length) {
    if(strings_idx + length > strings_length) {
      fprintf(stderr, "WordDBEncoded::ShiftString: (idx + length = %d) >= (length = %d)\n", strings_idx + length, strings_length);
      abort();
    }
    unsigned char* string = strings + strings_idx;
    strings_idx += length;
    return string;
  }
  
  void Put(WordBitCompress& stream);
  void Get(WordBitCompress& stream);
  
  unsigned int *values[WORD_CMPR_VAL_ARRAY_SIZE];
  int values_length[WORD_CMPR_VAL_ARRAY_SIZE];
  int values_idx[WORD_CMPR_VAL_ARRAY_SIZE];
  int values_size[WORD_CMPR_VAL_ARRAY_SIZE];

  unsigned char *strings;
  int strings_length;
  int strings_idx;
  int strings_size;
};

void WordDBEncoded::Put(WordBitCompress& stream)
{
  unsigned int count = 0;
  for(int i = 0; i < WORD_CMPR_VAL_ARRAY_SIZE; i++) {
    if(values_length[i] > 0) count++;
  }
  stream.WordBitStream::PutUint(count, WORD_CMPR_VAL_ARRAY_SIZE_BITS);

  for(int i = 0; i < WORD_CMPR_VAL_ARRAY_SIZE; i++) {
    if(values_length[i] > 0) {
      stream.WordBitStream::PutUint(i, WORD_CMPR_VAL_ARRAY_SIZE_BITS);      
      stream.PutUints(values[i], values_length[i]);
    }
  }

  stream.PutUchars(strings, strings_length);  
}

void WordDBEncoded::Get(WordBitCompress& stream)
{
  Clear();

  unsigned int count = 0;
  count = stream.WordBitStream::GetUint(WORD_CMPR_VAL_ARRAY_SIZE_BITS);

  for(unsigned int i = 0; i < count; i++) {
    unsigned int index = stream.WordBitStream::GetUint(WORD_CMPR_VAL_ARRAY_SIZE_BITS);
    values_length[index] = stream.GetUints(&values[index], &values_size[index]);
  }

  strings_length = stream.GetUchars(&strings, &strings_size);
}

//
// ******************** WordDBCompress implementation **************
//

#define WORD_CMPR_OVERHEAD	((int)sizeof(unsigned char))

WordDBCompress::WordDBCompress(WordContext* ncontext)
{
  cmprInfo = 0;
  context = ncontext;
  encoded = new WordDBEncoded();

  const Configuration& config = context->GetConfiguration();
  debug = config.Boolean("wordlist_compress_debug", 0);
  verbose = config.Value("wordlist_compress_verbose", 0);
}

WordDBCompress::~WordDBCompress()
{
  delete encoded;
}

int 
WordDBCompress::Compress(const  unsigned char *inbuff, int inbuff_length, unsigned char **outbuffp, int *outbuff_lengthp)
{
  //
  // Maximum output length can be WORD_CMPR_OVERHEAD more than input
  //
  int outbuff_length = inbuff_length + WORD_CMPR_OVERHEAD;
  unsigned char* outbuff = (unsigned char*)malloc(sizeof(unsigned char) * outbuff_length);

  *outbuffp = 0;
  *outbuff_lengthp = outbuff_length;

  if(outbuff == 0)
    return ENOMEM;

  int ret = 0;
  PAGE* pp = (PAGE*)inbuff;

  *outbuff = TYPE_TAGS(pp);

  switch(TYPE_TAGS(pp)) {
  case P_IBTREE|WORD_DB_INDEX:
  case P_LBTREE|WORD_DB_INDEX:
    ret = CompressBtree(inbuff, inbuff_length, outbuff, &outbuff_length);
    break;
  default:
    memcpy((char*)(outbuff + WORD_CMPR_OVERHEAD), (char*)inbuff, inbuff_length);
    break;
  }

  if(ret == 0) {
    *outbuffp = outbuff;
    *outbuff_lengthp = outbuff_length;
  } else {
    free(outbuff);
  }

  return ret;
}

int 
WordDBCompress::Uncompress(const unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int outbuff_length)
{
  int ret = 0;

  switch(*inbuff) {
  case P_IBTREE|WORD_DB_INDEX:
  case P_LBTREE|WORD_DB_INDEX:
    ret = UncompressBtree(inbuff, inbuff_length, outbuff, outbuff_length);
    break;
  default:
    memcpy((char*)outbuff, (char*)(inbuff + WORD_CMPR_OVERHEAD), outbuff_length);
    break;
  }

  return ret;
}

int 
WordDBCompress::CompressBtree(const  unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int *outbuff_lengthp)
{
  int ret = 0;
  PAGE *pp = (PAGE *)inbuff;

  if(verbose) fprintf(stderr, "WordDBCompress::CompressBtree: page %d\n", PGNO(pp));

  switch(TYPE(pp)) {
  case P_IBTREE:
    ret = CompressIBtree(inbuff, inbuff_length, outbuff, outbuff_lengthp);
    break;
  case P_LBTREE:
    ret = CompressLBtree(inbuff, inbuff_length, outbuff, outbuff_lengthp);
    break;
  }

  return ret;
}

//
// Return the length of the chars in b that are not common with a
// a = abcdef
// b = abcghi
//        ---
// return 3
//
static inline unsigned int suffixlength(const String& a, const String& b)
{
  const unsigned char* ap = (const unsigned char*)a.get();
  const unsigned char* bp = (const unsigned char*)b.get();
  int length = HtMIN(a.length(), b.length());

  int i;
  for(i = 0; i < length && ap[i] == bp[i]; i++)
    ;

  return b.length() - i;
}

int 
WordDBCompress::CompressIBtree(const  unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int *outbuff_lengthp)
{
  PAGE *pp = (PAGE *)inbuff;

  if(verbose > 5) DumpPage(inbuff);

  WordBitCompress stream(inbuff_length);

  stream.PutUint(pp->lsn.file, sizeof(pp->lsn.file) * 8);
  stream.PutUint(pp->lsn.offset, sizeof(pp->lsn.file) * 8);
  stream.PutUint(PGNO(pp), sizeof(PGNO(pp)) * 8);
  stream.PutUint(NUM_ENT(pp), sizeof(NUM_ENT(pp)) * 8);
  stream.PutUint(LEVEL(pp), sizeof(LEVEL(pp)) * 8);

  if(NUM_ENT(pp) > 0) {
    int i;
    WordKey key(context);
    WordKey previous_key(context);
    BINTERNAL* previous_e = 0;

    encoded->Clear();

    for (i = 0; i < NUM_ENT(pp); i++) {
      BINTERNAL* e = GET_BINTERNAL(pp, i);
      if(debug) {
	if(e->type != B_KEYDATA)
	  fprintf(stderr, "WordDBCompress::EncodeIBtree: unexpected type 0x%02x\n", e->type);
      }
      unsigned int changes = 0;

      if(e->len <= 0) {
	changes = WORD_CMPR_VAL_FLAGS_EMPTY;
	//
	// Structural data
	//
	encoded->PushValue(WORD_CMPR_VAL_PGNO, e->pgno);
	encoded->PushValue(WORD_CMPR_VAL_NRECS, e->nrecs);
      } else {
      
	key.Unpack((const char*)e->data, (int)e->len);

	if(previous_e != 0) {
	  int is_first_change = 1;
#if 0
	  //
	  // String
	  //
	  const String& word = key.GetWord();
	  const String& previous_word = previous_key.GetWord();
	  if(word != previous_word) {
	    unsigned int suffix_length = suffixlength(previous_word, word);
	    unsigned int prefix_length = word.length() - suffix_length;
	    encoded->PushValue(WORD_CMPR_VAL_PREFIX, prefix_length);
	    encoded->PushValue(WORD_CMPR_VAL_SUFFIX, suffix_length);
	    const char* suffix = word.get() + prefix_length;
	    encoded->PushString((const unsigned char*)suffix, suffix_length);
	    changes |= WORD_CMPR_VAL_FLAGS_STRING;
	    is_first_change = 0;
	  }
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    unsigned int value = key.Get(i);
	    unsigned int previous_value = previous_key.Get(i);
	    if(value != previous_value) {
	      if(is_first_change) {
		value -= previous_value;
		is_first_change = 0;
	      }
	      encoded->PushValue(i + WORD_CMPR_VAL_FIELDS, value);
	      changes |= WORD_CMPR_VAL_FLAGS_FIELD(i);
	    }
	  }
	  //
	  // Structural data
	  //
	  if(e->pgno != previous_e->pgno) {
	    encoded->PushValue(WORD_CMPR_VAL_PGNO, e->pgno);
	    changes |= WORD_CMPR_VAL_FLAGS_PGNO;
	  }
	  if(e->nrecs != previous_e->nrecs) {
	    encoded->PushValue(WORD_CMPR_VAL_NRECS, e->nrecs);
	    changes |= WORD_CMPR_VAL_FLAGS_NRECS;
	  }
	} else {
#if 0
	  //
	  // String
	  //
	  const String& word = key.GetWord();
	  encoded->PushValue(WORD_CMPR_VAL_SUFFIX, word.length());
	  encoded->PushString((const unsigned char*)word.get(), word.length());
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    encoded->PushValue(i + WORD_CMPR_VAL_FIELDS, key.Get(i));
	  }
	  //
	  // Structural data
	  //
	  encoded->PushValue(WORD_CMPR_VAL_PGNO, e->pgno);
	  encoded->PushValue(WORD_CMPR_VAL_NRECS, e->nrecs);
	}

	previous_e = e;
	previous_key = key;
      }

      encoded->PushValue(WORD_CMPR_VAL_FLAGS, changes);
    }

    encoded->Put(stream);
  }

  int outbuff_length = *outbuff_lengthp;
  if(stream.BuffLength() > (outbuff_length - WORD_CMPR_OVERHEAD)) {
    fprintf(stderr, "WordDBCompress::CompressIBtree: compressed length = %d > available length = %d\n", stream.BuffLength(), outbuff_length - WORD_CMPR_OVERHEAD);
    abort();
  }
  memcpy(outbuff + WORD_CMPR_OVERHEAD, stream.Buff(), stream.BuffLength());
  outbuff_length = WORD_CMPR_OVERHEAD + stream.BuffLength();

  if(debug) {
    unsigned char* tmp = (unsigned char*)malloc(inbuff_length);
    memset((char*)tmp, '\0', inbuff_length);
    UncompressIBtree(outbuff, outbuff_length, tmp, inbuff_length);
    if(DiffPage(inbuff, tmp)) {
      fprintf(stderr, "WordDBCompress::CompressIBtree: failed\n");
      DumpPage(inbuff);
      DumpPage(tmp);
      exit(1);
    }
    free(tmp);
  }

  *outbuff_lengthp = outbuff_length;

  return 0;
}

int 
WordDBCompress::CompressLBtree(const  unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int *outbuff_lengthp)
{
  PAGE *pp = (PAGE *)inbuff;

  if(verbose > 5) DumpPage(inbuff);

  WordBitCompress stream(inbuff_length);

  stream.PutUint(pp->lsn.file, sizeof(pp->lsn.file) * 8);
  stream.PutUint(pp->lsn.offset, sizeof(pp->lsn.file) * 8);
  stream.PutUint(PGNO(pp), sizeof(PGNO(pp)) * 8);
  stream.PutUint(PREV_PGNO(pp), sizeof(PREV_PGNO(pp)) * 8);
  stream.PutUint(NEXT_PGNO(pp), sizeof(NEXT_PGNO(pp)) * 8);
  stream.PutUint(NUM_ENT(pp), sizeof(NUM_ENT(pp)) * 8);
  stream.PutUint(LEVEL(pp), sizeof(LEVEL(pp)) * 8);

  if(NUM_ENT(pp) > 0) {
    int i;
    WordKey key(context);
    WordKey previous_key(context);
    WordRecord record(context);
    WordRecord previous_record(context);
    BKEYDATA* previous_key_e = 0;
    BKEYDATA* previous_record_e = 0;

    encoded->Clear();

    for (i = 0; i < NUM_ENT(pp); i += P_INDX) {
      unsigned int changes = 0;

      {
	BKEYDATA* key_e = GET_BKEYDATA(pp, i);
	if(debug) {
	  if(key_e->type != B_KEYDATA)
	    fprintf(stderr, "WordDBCompress::EncodeLBtree: unexpected type 0x%02x\n", key_e->type);
	  if(key_e->len <= 0)
	    fprintf(stderr, "WordDBCompress::EncodeLBtree: unexpected length <= 0\n");
	}

	key.Unpack((const char*)key_e->data, (int)key_e->len);

	if(previous_key_e != 0) {
	  int is_first_change = 1;
#if 0
	  //
	  // String
	  //
	  const String& word = key.GetWord();
	  const String& previous_word = previous_key.GetWord();
	  if(word != previous_word) {
	    unsigned int suffix_length = suffixlength(previous_word, word);
	    unsigned int prefix_length = word.length() - suffix_length;
	    encoded->PushValue(WORD_CMPR_VAL_PREFIX, prefix_length);
	    encoded->PushValue(WORD_CMPR_VAL_SUFFIX, suffix_length);
	    const char* suffix = word.get() + prefix_length;
	    encoded->PushString((const unsigned char*)suffix, suffix_length);
	    changes |= WORD_CMPR_VAL_FLAGS_STRING;
	    is_first_change = 0;
	  }
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    unsigned int value = key.Get(i);
	    unsigned int previous_value = previous_key.Get(i);
	    if(value != previous_value) {
	      if(is_first_change) {
		value -= previous_value;
		is_first_change = 0;
	      }
	      encoded->PushValue(WORD_CMPR_VAL_FIELDS + i, value);
	      changes |= WORD_CMPR_VAL_FLAGS_FIELD(i);
	    }
	  }
	} else {
#if 0
	  //
	  // String
	  //
	  const String& word = key.GetWord();
	  encoded->PushValue(WORD_CMPR_VAL_SUFFIX, word.length());
	  encoded->PushString((const unsigned char*)word.get(), word.length());
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    encoded->PushValue(i + WORD_CMPR_VAL_FIELDS, key.Get(i));
	  }
	}

	previous_key_e = key_e;
	previous_key = key;
      }
      {
	BKEYDATA* record_e = GET_BKEYDATA(pp, i + 1);
	if(debug) {
	  if(record_e->type != B_KEYDATA)
	    fprintf(stderr, "WordDBCompress::EncodeLBtree: unexpected type 0x%02x\n", record_e->type);
	}

	if(record_e->len <= 0) {
	  changes |= WORD_CMPR_VAL_FLAGS_RECORD_NO;
	} else {
	  record.Unpack((const char*)record_e->data, (int)record_e->len);

	  switch(record.type) {
	  case WORD_RECORD_DATA:
	    if(previous_record_e &&
	       record.info.data == previous_record.info.data) {
	      changes |= WORD_CMPR_VAL_FLAGS_RECORD_EQ;
	    } else {
	      encoded->PushValue(WORD_CMPR_VAL_RVALUE, record.info.data);
	    }
	    break;
	  case WORD_RECORD_STR:
	    changes |= WORD_CMPR_VAL_FLAGS_RECORD_STR;
	    if(previous_record_e &&
	       record.info.str == previous_record.info.str) {
	      changes |= WORD_CMPR_VAL_FLAGS_RECORD_EQ;
	    } else {
	      const String& string = record.info.str;
	      encoded->PushValue(WORD_CMPR_VAL_RLENGTH, string.length());
	      encoded->PushString((const unsigned char*)string.get(), string.length());
	    }
	    break;
	  default:
	    fprintf(stderr, "WordDBCompress::EncodeLBtree: unexpected record.type = %d\n", record.type);
	    break;
	  }

	  previous_record = record;
	  previous_record_e = record_e;
	}

      }
      encoded->PushValue(WORD_CMPR_VAL_FLAGS, changes);
    }

    encoded->Put(stream);
  }

  int outbuff_length = *outbuff_lengthp;
  if(stream.BuffLength() > (outbuff_length - WORD_CMPR_OVERHEAD)) {
    fprintf(stderr, "WordDBCompress::CompressLBtree: compressed length = %d > available length = %d\n", stream.BuffLength(), outbuff_length - WORD_CMPR_OVERHEAD);
    abort();
  }
  memcpy(outbuff + WORD_CMPR_OVERHEAD, stream.Buff(), stream.BuffLength());
  outbuff_length = WORD_CMPR_OVERHEAD + stream.BuffLength();

  if(debug) {
    unsigned char* tmp = (unsigned char*)malloc(inbuff_length);
    memset((char*)tmp, '\0', inbuff_length);
    UncompressLBtree(outbuff, outbuff_length, tmp, inbuff_length);
    if(DiffPage(inbuff, tmp)) {
      fprintf(stderr, "WordDBCompress::CompressLBtree: failed\n");
      DumpPage(inbuff);
      DumpPage(tmp);
      exit(1);
    }
    free(tmp);
  }

  *outbuff_lengthp = outbuff_length;

  return 0;
}

int 
WordDBCompress::UncompressBtree(const unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int outbuff_length)
{
  int ret = 0;
  switch((*inbuff) & TYPE_MASK) {
  case P_IBTREE:
    ret = UncompressIBtree(inbuff, inbuff_length, outbuff, outbuff_length);
    break;
  case P_LBTREE:
    ret = UncompressLBtree(inbuff, inbuff_length, outbuff, outbuff_length);
    break;
  }
  if(verbose) fprintf(stderr, "WordDBCompress::UncompressBtree: page %d\n", PGNO((PAGE*)outbuff));
  return ret;
}

/*
 * cdb___db_pitem --
 *	Put an item on a page.
 */
static void
cdb___db_pitem(PAGE *pagep, u_int32_t indx, u_int32_t nbytes, DBT *hdr, DBT *data)
{
  u_int8_t *p;

  HOFFSET(pagep) -= nbytes;
  pagep->inp[indx] = HOFFSET(pagep);
  ++NUM_ENT(pagep);

  p = P_ENTRY(pagep, indx);
  memcpy(p, hdr->data, hdr->size);
  if (data != NULL)
    memcpy(p + hdr->size, data->data, data->size);
}

int 
WordDBCompress::UncompressIBtree(const unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int outbuff_length)
{
  memset((char*)outbuff, '\0', outbuff_length);

  WordBitCompress stream(inbuff_length);
  stream.BuffSet(inbuff + WORD_CMPR_OVERHEAD, inbuff_length - WORD_CMPR_OVERHEAD);

  PAGE *pp = (PAGE *)outbuff;

  TYPE_TAGS(pp) = *inbuff;
  HOFFSET(pp) = outbuff_length;
  short entry_count = 0;
  pp->lsn.file = stream.GetUint(sizeof(pp->lsn.file) * 8);
  pp->lsn.offset = stream.GetUint(sizeof(pp->lsn.offset) * 8);
  PGNO(pp) = stream.GetUint(sizeof(PGNO(pp)) * 8);
  entry_count = stream.GetUint(sizeof(NUM_ENT(pp)) * 8);
  LEVEL(pp) = stream.GetUint(sizeof(LEVEL(pp)) * 8);
  NEXT_PGNO(pp) = PREV_PGNO(pp) = 0;

  if(entry_count > 0) {
    int i;
    String packed;
    BINTERNAL previous_bi;
    BINTERNAL bi;
    DBT hdr;
    hdr.data = &bi;
    hdr.size = SSZA(BINTERNAL, data);
    DBT data;
    WordKey key(context);
    WordKey previous_key(context);
    String word;

    encoded->Get(stream);

    for(i = 0; i < entry_count; i++) {
      unsigned int changes = encoded->ShiftValue(WORD_CMPR_VAL_FLAGS);

      memset((char*)&bi, '\0', sizeof(BINTERNAL));

      if(changes & WORD_CMPR_VAL_FLAGS_EMPTY) {
	packed.trunc();
	//
	// Structural data
	//
	bi.pgno = encoded->ShiftValue(WORD_CMPR_VAL_PGNO);
	bi.nrecs = encoded->ShiftValue(WORD_CMPR_VAL_NRECS);
      } else {
	key.Clear();

	if(!previous_key.Empty()) {
	  int is_first_change = 1;
#if 0
	  //
	  // String
	  //
	  if(changes & WORD_CMPR_VAL_FLAGS_STRING) {
	    unsigned int suffix_length = encoded->ShiftValue(WORD_CMPR_VAL_SUFFIX);
	    unsigned int prefix_length = encoded->ShiftValue(WORD_CMPR_VAL_PREFIX);
	    unsigned char* suffix = encoded->ShiftString(suffix_length);
	    word.set(previous_key.GetWord().get(), prefix_length);
	    word.append((const char*)suffix, suffix_length);
	    key.SetWord(word);
	    is_first_change = 0;
	  } else {
	    key.SetWord(previous_key.GetWord());
	  }
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int j = 0; j < nfields; j++) {
	    if(changes & WORD_CMPR_VAL_FLAGS_FIELD(j)) {
	      unsigned int value = encoded->ShiftValue(j + WORD_CMPR_VAL_FIELDS);
	      if(is_first_change) {
		value += previous_key.Get(j);
		is_first_change = 0;
	      }
	      key.Set(j, value);
	    } else {
	      key.Set(j, previous_key.Get(j));
	    }
	  }
	  //
	  // Structural data
	  //
	  if(changes & WORD_CMPR_VAL_FLAGS_PGNO) {
	    bi.pgno = encoded->ShiftValue(WORD_CMPR_VAL_PGNO);
	  } else {
	    bi.pgno = previous_bi.pgno;
	  }
	  if(changes & WORD_CMPR_VAL_FLAGS_NRECS) {
	    bi.nrecs = encoded->ShiftValue(WORD_CMPR_VAL_NRECS);
	  } else {
	    bi.nrecs = previous_bi.nrecs;
	  }
	} else {
#if 0
	  //
	  // String
	  //
	  unsigned int string_length = encoded->ShiftValue(WORD_CMPR_VAL_SUFFIX);
	  unsigned char* string = encoded->ShiftString(string_length);
	  key.SetWord((const char*)string, string_length);
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    key.Set(i, encoded->ShiftValue(i + WORD_CMPR_VAL_FIELDS));
	  }
	  //
	  // Structural data
	  //
	  bi.pgno = encoded->ShiftValue(WORD_CMPR_VAL_PGNO);
	  bi.nrecs = encoded->ShiftValue(WORD_CMPR_VAL_NRECS);
	}

	key.Pack(packed);
      }
      data.data = packed.get();
      data.size = packed.length();
      bi.len = packed.length();
      bi.type = B_KEYDATA;
      
      //
      // Insert entry in page
      //
      cdb___db_pitem(pp, i, BINTERNAL_SIZE(bi.len), &hdr, &data);

      //
      // Current becomes previous
      //
      previous_bi = bi;
      previous_key = key;
    }
  }

  if(debug) {
    if(entry_count != NUM_ENT(pp))
      fprintf(stderr, "WordDBCompress::UncompressIBtree: pgno %d: NUM_ENT(pp) = %d is different than entry_count = %d\n", PGNO(pp), NUM_ENT(pp), entry_count);
  }
  return 0;
}

int 
WordDBCompress::UncompressLBtree(const unsigned char *inbuff, int inbuff_length, unsigned char *outbuff, int outbuff_length)
{
  memset((char*)outbuff, '\0', outbuff_length);

  WordBitCompress stream(inbuff_length);
  stream.BuffSet(inbuff + WORD_CMPR_OVERHEAD, inbuff_length - WORD_CMPR_OVERHEAD);

  PAGE *pp = (PAGE *)outbuff;

  TYPE_TAGS(pp) = *inbuff;
  HOFFSET(pp) = outbuff_length;
  short entry_count = 0;
  pp->lsn.file = stream.GetUint(sizeof(pp->lsn.file) * 8);
  pp->lsn.offset = stream.GetUint(sizeof(pp->lsn.offset) * 8);
  PGNO(pp) = stream.GetUint(sizeof(PGNO(pp)) * 8);
  PREV_PGNO(pp) = stream.GetUint(sizeof(PREV_PGNO(pp)) * 8);
  NEXT_PGNO(pp) = stream.GetUint(sizeof(NEXT_PGNO(pp)) * 8);
  entry_count = stream.GetUint(sizeof(NUM_ENT(pp)) * 8);
  LEVEL(pp) = stream.GetUint(sizeof(LEVEL(pp)) * 8);

  if(entry_count > 0) {
    int i;
    String packed;
    BKEYDATA bk;
    DBT hdr;
    hdr.data = &bk;
    hdr.size = SSZA(BKEYDATA, data);
    DBT data;
    WordKey key(context);
    WordKey previous_key(context);
    WordRecord record(context);
    WordRecord previous_record(context);
    int previous_record_p = 0;
    String word;
  
    encoded->Get(stream);

    for(i = 0; i < entry_count; i += P_INDX) {
      unsigned int changes = encoded->ShiftValue(WORD_CMPR_VAL_FLAGS);

      //
      // Key
      //
      {
	memset((char*)&bk, '\0', sizeof(BKEYDATA));

	key.Clear();

	if(!previous_key.Empty()) {
	  int is_first_change = 1;
#if 0
	  //
	  // String
	  //
	  if(changes & WORD_CMPR_VAL_FLAGS_STRING) {
	    unsigned int suffix_length = encoded->ShiftValue(WORD_CMPR_VAL_SUFFIX);
	    unsigned int prefix_length = encoded->ShiftValue(WORD_CMPR_VAL_PREFIX);
	    unsigned char* suffix = encoded->ShiftString(suffix_length);
	    word.set(previous_key.GetWord().get(), prefix_length);
	    word.append((const char*)suffix, suffix_length);
	    key.SetWord(word);
	    is_first_change = 0;
	  } else {
	    key.SetWord(previous_key.GetWord());
	  }
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int j = 0; j < nfields; j++) {
	    if(changes & WORD_CMPR_VAL_FLAGS_FIELD(j)) {
	      unsigned int value = encoded->ShiftValue(j + WORD_CMPR_VAL_FIELDS);
	      if(is_first_change) {
		value += previous_key.Get(j);
		is_first_change = 0;
	      }
	      key.Set(j, value);
	    } else {
	      key.Set(j, previous_key.Get(j));
	    }
	  }
	} else {
#if 0
	  //
	  // String
	  //
	  unsigned int string_length = encoded->ShiftValue(WORD_CMPR_VAL_SUFFIX);
	  unsigned char* string = encoded->ShiftString(string_length);
	  key.SetWord((const char*)string, string_length);
#endif
	  //
	  // Fields
	  //
	  int nfields = key.NFields();
	  for(int i = 0; i < nfields; i++) {
	    key.Set(i, encoded->ShiftValue(i + WORD_CMPR_VAL_FIELDS));
	  }
	}

	key.Pack(packed);

	data.data = packed.get();
	data.size = packed.length();
	bk.len = packed.length();
	bk.type = B_KEYDATA;
      
	//
	// Insert entry in page
	//
	cdb___db_pitem(pp, i, BKEYDATA_SIZE(bk.len), &hdr, &data);

	previous_key = key;
      }
      //
      // Record
      //
      {
	memset((char*)&bk, '\0', sizeof(BKEYDATA));

	if(changes & WORD_CMPR_VAL_FLAGS_RECORD_NO) {
	  packed.trunc();
	} else {
	  if(changes & WORD_CMPR_VAL_FLAGS_RECORD_STR) {
	    record.type = WORD_RECORD_STR;
	    if(previous_record_p &&
	       (changes & WORD_CMPR_VAL_FLAGS_RECORD_EQ)) {
	      record.info.str = previous_record.info.str;
	    } else {
	      unsigned int string_length = encoded->ShiftValue(WORD_CMPR_VAL_RLENGTH);
	      unsigned char* string = encoded->ShiftString(string_length);
	      record.info.str.set((const char*)string, string_length);
	    }
	  } else {
	    record.type = WORD_RECORD_DATA;
	  
	    if(previous_record_p &&
	       (changes & WORD_CMPR_VAL_FLAGS_RECORD_EQ)) {
	      record.info.data = previous_record.info.data;
	    } else {
	      record.info.data = encoded->ShiftValue(WORD_CMPR_VAL_RVALUE);
	    }
	  }

	  record.Pack(packed);

	  previous_record = record;
	  previous_record_p = 1;
	}
	data.data = packed.get();
	data.size = packed.length();
	bk.len = packed.length();
	bk.type = B_KEYDATA;
      
	//
	// Insert entry in page
	//
	cdb___db_pitem(pp, i + 1, BKEYDATA_SIZE(bk.len), &hdr, &data);
      }
    }
  }

  if(debug) {
    if(entry_count != NUM_ENT(pp))
      fprintf(stderr, "WordDBCompress::UncompressLBtree: pgno %d: NUM_ENT(pp) = %d is different than entry_count = %d\n", PGNO(pp), NUM_ENT(pp), entry_count);
  }

  return 0;
}

DB_CMPR_INFO* WordDBCompress::CmprInfo()
{
  DB_CMPR_INFO *cmpr_info = new DB_CMPR_INFO;

  cmpr_info->user_data = (void *)this;
  cmpr_info->compress = WordDBCompress_compress_c;
  cmpr_info->uncompress = WordDBCompress_uncompress_c;
  cmpr_info->coefficient = 3;
  cmpr_info->max_npages = 9;
  
  cmprInfo = cmpr_info;
  
  return cmpr_info;
}

void
WordDBCompress::DumpPage(const unsigned char* page) const
{
  PAGE* pp = (PAGE*)page;

  fprintf(stderr, "--------------------------------------------------\n");
  fprintf(stderr, "pgno = %d ", PGNO(pp));
  fprintf(stderr, "lsn.file = %d ", pp->lsn.file);
  fprintf(stderr, "lsn.offset = %d ", pp->lsn.offset);
  fprintf(stderr, "prev_pgno = %d ", PREV_PGNO(pp));
  fprintf(stderr, "next_pgno = %d\n", NEXT_PGNO(pp));
  fprintf(stderr, "entries = %d ", NUM_ENT(pp));
  fprintf(stderr, "hf_offset = %d ", HOFFSET(pp));
  fprintf(stderr, "level = %d ", LEVEL(pp));
  fprintf(stderr, "type = %d\n", TYPE(pp));
  fprintf(stderr, "tags = 0x%02x\n", TAGS(pp));
  fprintf(stderr, "freespace = %d bytes from byte %d to byte %d\n", P_FREESPACE(pp), LOFFSET(pp), HOFFSET(pp));

  int i;
  for(i = 0; i < NUM_ENT(pp); i++) {
    fprintf(stderr, "index = %d, ", pp->inp[i]);
    unsigned char* data = 0;
    int data_length = 0;
    switch(TYPE(pp)) {
    case P_IBTREE:
      {
	BINTERNAL* e = GET_BINTERNAL(pp, i);
	fprintf(stderr, "len = %d, type = %d, pgno = %d, nrecs = %d\n", e->len, e->type, e->pgno, e->nrecs);
	data = (unsigned char*)e->data;
	data_length = e->len;
      }
      break;
    case P_LBTREE:
      {
	BKEYDATA* e = GET_BKEYDATA(pp, i);
	fprintf(stderr, "len = %d, type = %d\n", e->len, e->type);
	data = (unsigned char*)e->data;
	data_length = e->len;
      }
      break;
    }
    if(data_length > 0) {
      int j;
      for(j = 0; j < data_length; j++) {
	fprintf(stderr, "(%d) ", data[j]);
      }
      fprintf(stderr, "\n");
    }
  }
}

int
WordDBCompress::DiffPage(const unsigned char* first, const unsigned char* second) const
{
  PAGE* p1 = (PAGE*)first;
  PAGE* p2 = (PAGE*)second;

  if(TAGS(p1) != TAGS(p2)) return 1;
  if(TYPE(p1) != TYPE(p2)) return 1;
  if(PGNO(p1) != PGNO(p2)) return 1;
  if(p1->lsn.file != p2->lsn.file) return 1;
  if(p1->lsn.offset != p2->lsn.offset) return 1;
  if(TYPE(p1) == P_LBTREE) {
    if(PREV_PGNO(p1) != PREV_PGNO(p2)) return 1;
    if(NEXT_PGNO(p1) != NEXT_PGNO(p2)) return 1;
  }
  if(NUM_ENT(p1) != NUM_ENT(p2)) return 1;
  if(HOFFSET(p1) != HOFFSET(p2)) return 1;
  if(LEVEL(p1) != LEVEL(p2)) return 1;

  int i;
  for(i = 0; i < NUM_ENT(p1); i++) {
    unsigned char* data1 = 0;
    int data1_length = 0;
    unsigned char* data2 = 0;
    int data2_length = 0;
    switch(TYPE(p1)) {
    case P_IBTREE:
      {
	BINTERNAL* e1 = GET_BINTERNAL(p1, i);
	BINTERNAL* e2 = GET_BINTERNAL(p2, i);
	if(e1->len != e2->len) return 1;
	if(e1->type != e2->type) return 1;
	if(e1->pgno != e2->pgno) return 1;
	if(e1->nrecs != e2->nrecs) return 1;
	data1 = (unsigned char*)e1->data;
	data1_length = e1->len;
	data2 = (unsigned char*)e2->data;
	data2_length = e2->len;
      }
      break;
    case P_LBTREE:
      {
	BKEYDATA* e1 = GET_BKEYDATA(p1, i);
	BKEYDATA* e2 = GET_BKEYDATA(p2, i);
	if(e1->len != e2->len) return 1;
	if(e1->type != e2->type) return 1;
	data1 = (unsigned char*)e1->data;
	data1_length = e1->len;
	data2 = (unsigned char*)e2->data;
	data2_length = e2->len;
      }
      break;
    }
    if(data1_length > 0) {
      int j;
      for(j = 0; j < data1_length; j++) {
	if(data1[j] != data2[j]) return 1;
      }
    }
  }
  return 0;
}
