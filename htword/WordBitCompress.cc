//
// WordBitCompress.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordBitCompress.cc,v 1.1.2.17 2000/09/14 03:13:24 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#include"WordBitCompress.h"
#include"HtMaxMin.h"

inline static int bitcount(unsigned int maxval)
{
    unsigned int mv = maxval;
    int nbits;
    for (nbits = 0; mv; nbits++)
	mv >>= 1;
    return nbits;
}

// log in base 2 of v
// log2(0) -> -1
// log2(1) ->  0
// log2(2) ->  1
// log2(4) ->  2
// ...
// log2(8) ->  3
// log2(7) ->  2
int
log2(unsigned int v)
{
    int res;
    for(res=-1;v;res++){v>>=1;}
    return(res);
}

// compute 2^x
#define pow2(x) (1<<(x))

// **************************************************
// *************** VlengthCoder   *******************
// **************************************************
//
// Compress values into a bitstream based on their probability distribution
// The probability distribution is reduced to a number of intervals.
// Each  interval (generally)  has the same probability of occuring
// values are then coded by:  interval_number position_inside_interval
// this can be seen as modified version of shanon-fanno encoding
//
// Here are some aproximate calculation for estimating final coded size: 
//
// n number of entries to code
// nbits maximum size in bits of entries to code
//
// SUM_interval_bit_sizes -> depends on probability dist
// total_size = table_size + coded_size
// table_size = 2^interval_bits * NBITS_NBITS_VAL
// coded_size = n * (interval_bits + SUM_interval_bit_sizes / 2^interval_bits )
//
// example1: flat probability distribution :
// SUM_interval_bit_sizes = 2^interval_bits * log2( 2^nbits / 2^interval_bits) = 2^interval_bits * ( nbits - interval_bits )
// => coded_size = n * ( interval_bits + nbits - interval_bits ) = n*nbits !!
// => coded_size is the same as if we used no compression
//    this is normal, because it is not possible to compress random data
//
// example2: probability all focused in first interval except for one entry
// SUM_interval_bit_sizes  = 1 + nbits
// the computations above are not valid because of integer roundofs
// => coded_size would actually be = n *  1 + nbits 
// (but the code needs a few cleanups to obtain this value) 
//

//
// Representation of an unsigned int interval [low low+size]
//
class WordInterval {
public:
  inline void SizeFromBits() {
    size = ((nbits > 0 ? pow2(nbits - 1) : 0));
  }

  //
  // Number of bits to code values in the range
  // [0 size]
  //
  int nbits;
  //
  // Size of the interval
  //
  unsigned int size;
  //
  // Lowest bound of the interval
  //
  unsigned int low;
};

class VlengthCoder {
public:
  //
  // Constructor. 
  //
  VlengthCoder(WordBitStream & nbs);

  ~VlengthCoder() {
    if(intervals) delete [] intervals;
  }

  void PutUints(unsigned int *vals, int n);
  void PutUintsPrepare(unsigned int *vals, int n);
  void GetUints(unsigned int *vals, int n);

  // compress and insert a value into the bitstream
  inline void PutUint(unsigned int val) {
    unsigned int low = 0;
    int interval = 0;
    FindInterval(val, interval, low);

    bs.PutUint(interval, interval_bits);	// store interval

    const int bitsremaining = (intervals[interval].nbits > 0 ? intervals[interval].nbits - 1 : 0);
    val -= low;
    bs.PutUint(val, bitsremaining);
  }

  // get and uncompress  a value from  the bitstream
  inline unsigned int GetUint() {
    int interval = bs.GetUint(interval_bits);	// get interval
    const int bitsremaining =
      (intervals[interval].nbits > 0 ? intervals[interval].nbits - 1 : 0);
    unsigned int val = bs.GetUint(bitsremaining);
    val += intervals[interval].low;
    return (val);
  }

  // find interval where value v resides
  // (very unusual implementation of binary search)
  inline void FindInterval(const unsigned int v, int &interval, unsigned int &low) {
    int i0 = 0;
    int i1 = nintervals;
    int i;
    for (;;) {
      if (i1 == i0 + 1) {
	break;
      }
      i = (i0 + i1) >> 1;
      low = intervals[i].low;
      if (v < low) {
	i1 = i;
	continue;
      } else {
	i0 = i;
	continue;
      }

    }

    low = intervals[i0].low;
    interval = i0;
  }

  void GenerateLowBoundaries(WordInterval *intervals, int nintervals);

private:
  int interval_bits;		// number of bits needed to code an index
                                // of the intervals data member (maximum 
                                // value of index is nintervals - 1).

  WordInterval *intervals;
  int nintervals;		// number of intervals

  WordBitStream & bs;
};

VlengthCoder::VlengthCoder(WordBitStream & nbs):bs(nbs)
{
    interval_bits = 0;
    nintervals = 0;
    intervals = NULL;
}

// quick sort compare function (for unsigned int's)
static int qsort_uint_cmp(const void *a, const void *b)
{
    if ((*((unsigned int *) a)) > (*((unsigned int *) b)))
	return 1;
    else if ((*((unsigned int *) a)) < (*((unsigned int *) b)))
	return -1;
    else
	return 0;
//      return 
//      (*((unsigned int *)a)) -
//      (*((unsigned int *)b))   ;
}

// quick sort an array of unsigned int's
static void qsort_uint(unsigned int *v, int n)
{
    qsort((void *) v, (unsigned int) n, sizeof(unsigned int), &qsort_uint_cmp);
}

void VlengthCoder::PutUints(unsigned int *vals, int n)
{
  PutUintsPrepare(vals, n);

  int i;
  bs.PutUint(interval_bits, WORD_CMPR_LOG32_BITS);
  for (i = 0; i < nintervals; i++) {
    bs.PutUint(intervals[i].nbits, WORD_CMPR_LOG32_BITS);
  }

  for (i = 0; i < n; i++)
    PutUint(vals[i]);
}

void VlengthCoder::PutUintsPrepare(unsigned int *vals, int n)
{
    unsigned int *sorted = new unsigned int[n];
    memcpy((void *) sorted, (void *) vals, n * sizeof(unsigned int));
    qsort_uint(sorted, n);

    int nbits = bitcount(sorted[n - 1]);

    // **** heuristics to determine best interval_bits
    // The interval table should not be larger than 1/10 of the 
    // data. force table size to be less than 1/10 of the maximum coded size
    interval_bits = bitcount((n * nbits) / (10 * WORD_CMPR_LOG32_BITS));
    // sanity
    if (interval_bits >= nbits) {
	interval_bits = nbits - 1;
    }
    // interval_bits at least 1
    if (interval_bits < 1) {
	interval_bits = 1;
    }

    nintervals = (1 << interval_bits);
    int i;

    // + 1 because .low will be set for the past-to-last element
    // algorithmic glitch ? 
    intervals = new WordInterval[nintervals + 1];

    // find split boundaires
    unsigned int lboundary = 0;
    unsigned int boundary;
    for (i = 0; i < nintervals - 1; i++) {
	boundary = sorted[(n * (i + 1)) / nintervals];
	intervals[i].nbits = 1 + log2(boundary - lboundary);
	intervals[i].SizeFromBits();
	lboundary += intervals[i].size;
    }
    boundary = sorted[n - 1];
    intervals[i].nbits = 1 + log2(boundary - lboundary) + 1;
    intervals[i].SizeFromBits();

    GenerateLowBoundaries(intervals, nintervals);

    delete [] sorted;
}

void VlengthCoder::GetUints(unsigned int *vals, int n)
{
    int i;
    interval_bits = bs.GetUint(WORD_CMPR_LOG32_BITS);
    nintervals = pow2(interval_bits);

    // + 1 because .low will be set for the past-to-last element
    // algorithmic glitch ? 
    intervals = new WordInterval[nintervals + 1];

    for (i = 0; i < nintervals; i++) {
	intervals[i].nbits = bs.GetUint(WORD_CMPR_LOG32_BITS);
	intervals[i].SizeFromBits();
    }
    GenerateLowBoundaries(intervals, nintervals);

    for (i = 0; i < n; i++)
	vals[i] = GetUint();
}

void VlengthCoder::GenerateLowBoundaries(WordInterval *intervals, int nintervals)
{
    unsigned int low = 0;
    for (int j = 0; j <= nintervals; j++) {
	intervals[j].low = low;
	if (j < nintervals) {
	    low += intervals[j].size;
	}
    }
}

// **************************************************
// *************** WordBitStream  ***********************
// **************************************************

void 
WordBitStream::PutUint(unsigned int v, int n)
{
  if(freezeon) {
    freeze_bitcount += n;
    return;
  }
    
  if(n <= 0) return;

  int relative_bitpos = bitpos & 0x07;
  // simplest case it all fits in the current byte
  if(relative_bitpos + n < 8) {
    // would be safer to mask v to make
    // sure no spurious bits are inserted if there is garbadge in v
    buff[buff_idx] |= (v << relative_bitpos);
    BitposIncr(n);
    return;
  } else {
    const int central = ((relative_bitpos + n) >> 3) - 1;

    // put first
    const int bits_in_first_byte = 8 - relative_bitpos;
    buff[buff_idx] |= ((v & 0xff) << relative_bitpos) & 0xff;
    BitposIncr(bits_in_first_byte);
    v >>= bits_in_first_byte;

    // put central
    for(int i = central; i ;i--) {
      buff[buff_idx] = v & 0xff ;
      BitposIncr(8);
      v >>= 8;	    
    }

    // put last
    const int bits_in_last_byte = n - ((central << 3) + bits_in_first_byte);

    if(bits_in_last_byte > 0) {
      buff[buff_idx] = v & ((1 << bits_in_last_byte) - 1);
      BitposIncr(bits_in_last_byte);
    }
  }
}

unsigned int 
WordBitStream::GetUint(int n)
{
  if(n <= 0) return 0;

  unsigned int res = 0;

  int relative_bitpos = bitpos & 0x07;

  if(relative_bitpos + n < 8) {
    // simplest case it all fits in the current byte
    res = (buff[bitpos >> 3] >> relative_bitpos) & ((1 << n) - 1);
    bitpos += n;
    return res;
  } else {
    int bytepos = bitpos >> 3;
    const int central = ((relative_bitpos + n) >> 3) - 1;

    // put first
    res = (buff[bytepos] >> relative_bitpos) & 0xff;
    const int bits_in_first_byte = 8 - relative_bitpos;
    bytepos++;

    // put central
    if(central) {
      unsigned int tmp_res = 0;
      for(int i = central - 1; i >= 0; i--) {
	tmp_res |= buff[bytepos + i] & 0xff;
	if(i) tmp_res <<= 8;
      }
      bytepos += central;
      res |= tmp_res << bits_in_first_byte;
    }
    
    // put last
    const int bits_in_last_byte = n - ((central << 3) + bits_in_first_byte);
    if(bits_in_last_byte > 0) {
      res |= ((unsigned int)(buff[bytepos] & ((1 << bits_in_last_byte) - 1))) << (bits_in_first_byte + ((bytepos - (bitpos>>3) - 1) << 3));
    }

    bitpos+=n;
    return res;
  }
}

void 
WordBitStream::PutZone(unsigned char *vals, int n)
{
  int limit = (n + 7) / 8;
  for(int i = 0; i < limit; i++) {
    const int remains = n - (8 * i);
    PutUint(vals[i], remains > 8 ? 8 : remains);
  }
}

void 
WordBitStream::GetZone(unsigned char *vals, int n)
{
  int limit = (n + 7) / 8;
  for(int i = 0; i < limit; i++) {
    const int remains = n - (8 * i);
    vals[i] = GetUint(remains > 8 ? 8 : remains);
  }
}

// **************************************************
// *************** WordBitCompress ***********************
// **************************************************

//
// Count the minimum number of bits to consider to code
// the value in binary.
// 0 -> 0
// 1 -> 1
// 2 -> 2
// 3 -> 2
// 4 -> 3
// 5 -> 3
// 6 -> 3
// ...
// 

void 
WordBitCompress::PutUint(unsigned int v, int n)
{
    int nbits = bitcount(v);
    WordBitStream::PutUint(nbits, bitcount(n));
    if(nbits)
      WordBitStream::PutUint(v, nbits);
}

unsigned int 
WordBitCompress::GetUint(int n)
{
    int nbits = WordBitStream::GetUint(bitcount(n));
    if(!nbits)
      return 0;
    else
      return WordBitStream::GetUint(nbits);
}

int 
WordBitCompress::PutUints(unsigned int *vals, int n)
{
  int cpos = Length();
    
  if(n >= (1 << WORD_CMPR_NBITS_NVALS)) {
    fprintf(stderr, "WordBitCompress::PutUints: : overflow: n >= %d\n", (1 << WORD_CMPR_NBITS_NVALS));
    abort();
  }

  PutUint(n, WORD_CMPR_NBITS_NVALS);
  if(n == 0)
    return Length() - cpos;

  int max_nbits = bitcount(HtMaxMin::max_v(vals, n));

  int sdecr;
  int sfixed;

  //
  // Only bother to compare the Decr and Fixed compression
  // performances if there are more than 15 values involved and that
  // the max of these values is encoded on more than 3 bits.
  //
  if(n > 15 && max_nbits > 3) {
    Freeze();
    PutUintsDecr(vals, n);
    sdecr = Length();
    UnFreeze();

    Freeze();
    PutUintsFixed(vals, n);
    sfixed = Length();
    UnFreeze();
  } else {
    //
    // Set to arbitrary values so that sdecr > sfixed, hence
    // Fixed scheme will be chosen;
    //
    sdecr = 4242;
    sfixed = 0;
  }

  //
  // Encode the array using the best model (the one that
  // takes less space).
  //
  if(sdecr < sfixed) {
    WordBitStream::PutUint(WORD_CMPR_MODEL_DECR, WORD_CMPR_MODEL_BITS);
    PutUintsDecr(vals, n);
  } else {
    WordBitStream::PutUint(WORD_CMPR_MODEL_FIXED, WORD_CMPR_MODEL_BITS);
    PutUintsFixed(vals,n);
  }

  return Length() - cpos;
}

int 
WordBitCompress::GetUints(unsigned int **valsp)
{
    int n = GetUint(WORD_CMPR_NBITS_NVALS);

    if(!n) {
      *valsp = NULL;
      return 0;
    }

    unsigned int *vals = new unsigned int[n];

    int model = WordBitStream::GetUint(WORD_CMPR_MODEL_BITS);

    switch(model) {
    case WORD_CMPR_MODEL_DECR:
      GetUintsDecr(vals, n);
      break;
    case WORD_CMPR_MODEL_FIXED:
      GetUintsFixed(vals, n);
      break;
    default:
      fprintf(stderr, "WordBitCompress::GetUints invalid compression model %d\n", model);
      abort();
      break;
    }

    *valsp = vals;
    
    return n;
}

int 
WordBitCompress::GetUints(unsigned int **valsp, int* vals_sizep)
{
    int n = GetUint(WORD_CMPR_NBITS_NVALS);

    if(!n) {
      return 0;
    }

    while(n >= *vals_sizep) {
      (*vals_sizep) *= 2;
      (*valsp) = (unsigned int*)realloc(*valsp, (*vals_sizep) * sizeof(unsigned int));
    }

    int model = WordBitStream::GetUint(WORD_CMPR_MODEL_BITS);

    switch(model) {
    case WORD_CMPR_MODEL_DECR:
      GetUintsDecr(*valsp, n);
      break;
    case WORD_CMPR_MODEL_FIXED:
      GetUintsFixed(*valsp, n);
      break;
    default:
      fprintf(stderr, "WordBitCompress::GetUints invalid compression model %d\n", model);
      abort();
      break;
    }

    return n;
}

int WordBitCompress::PutUchars(unsigned char *vals, int n)
{
    int cpos = Length();

    if(n >= (1 << WORD_CMPR_NBITS_NVALS)) {
      fprintf(stderr, "WordBitCompress::PutUchars: : overflow: n >= %d\n", (1 << WORD_CMPR_NBITS_NVALS));
      abort();
    }

    PutUint(n, WORD_CMPR_NBITS_NVALS);
    if (n == 0) {
	return 0;
    }

    int max_nbits = bitcount(HtMaxMin::max_v(vals, n));

    if(max_nbits >= (1 << WORD_CMPR_LOG8_BITS)) {
      fprintf(stderr, "WordBitCompress::PutUchars: : overflow: max_nbits >= %d\n", (1 << WORD_CMPR_LOG8_BITS));
      abort();
    }

    WordBitStream::PutUint(max_nbits, WORD_CMPR_LOG8_BITS);
    
    for(int i = 0; i < n; i++) {
        WordBitStream::PutUint(vals[i] & 0xff, max_nbits);
#if 0
	unsigned char v = vals[i];
	for (int j = 0; j < max_nbits; j++) {
	  Put(v & (1 << j));
	}
#endif
    }
    return Length() - cpos;
}

int 
WordBitCompress::GetUchars(unsigned char **valsp)
{
    int n = GetUint(WORD_CMPR_NBITS_NVALS);
    if (!n) {
	*valsp = NULL;
	return 0;
    }
    
    int nbits = WordBitStream::GetUint(WORD_CMPR_LOG8_BITS);
    int i;
    unsigned char *vals = new unsigned char[n];

    for (i = 0; i < n; i++)
	vals[i] = WordBitStream::GetUint(nbits);

    *valsp = vals;
    return n;
}

int 
WordBitCompress::GetUchars(unsigned char **valsp, int *vals_sizep)
{
    int n = GetUint(WORD_CMPR_NBITS_NVALS);
    if (!n) {
	return 0;
    }
    
    while(n >= *vals_sizep) {
      (*vals_sizep) *= 2;
      (*valsp) = (unsigned char*)realloc(*valsp, (*vals_sizep) * sizeof(unsigned char));
    }

    int nbits = WordBitStream::GetUint(WORD_CMPR_LOG8_BITS);
    int i;

    for (i = 0; i < n; i++)
	(*valsp)[i] = WordBitStream::GetUint(nbits);

    return n;
}

void WordBitCompress::PutUintsFixed(unsigned int *vals, int n)
{
    int max_nbits = bitcount(HtMaxMin::max_v(vals, n));

    if(max_nbits >= (1 << WORD_CMPR_LOG32_BITS)) {
      fprintf(stderr, "WordBitCompress::PutUintsFixed: : overflow: max_nbits >= %d\n", (1 << WORD_CMPR_LOG32_BITS));
      abort();
    }

    PutUint(max_nbits, WORD_CMPR_LOG32_BITS);

    for (int i = 0; i < n; i++)
	WordBitStream::PutUint(vals[i], max_nbits);
}

void WordBitCompress::GetUintsFixed(unsigned int *vals, int n)
{
    int nbits = GetUint(WORD_CMPR_LOG32_BITS);

    int i;
    for (i = 0; i < n; i++)
	vals[i] = WordBitStream::GetUint(nbits);
}

void WordBitCompress::PutUintsDecr(unsigned int *vals, int n)
{
    VlengthCoder coder(*this);
    coder.PutUints(vals, n);
}

void WordBitCompress::GetUintsDecr(unsigned int *vals, int n)
{
    VlengthCoder coder(*this);
    coder.GetUints(vals, n);
}
