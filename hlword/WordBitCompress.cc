//
// WordBitCompress.cc
//
// BitStream: put and get bits into a buffer
//           *tagging:  add tags to keep track of the position of data 
//                      inside the bitstream for debuging purposes.
//           *freezing: saves current position. further inserts in the BitStream
//                      aren't really done. This way you can try different
//                      compression algorithms and chose the best.
//
// Compressor: BitStream with extended compression fuctionalities
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordBitCompress.cc,v 1.5 2004/05/28 13:15:26 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#include"WordBitCompress.h"

// ******** HtVector_byte (implementation)
#define GType byte
#define HtVectorGType HtVector_byte
#include "HtVectorGenericCode.h"

// ******** HtVector_charptr (implementation)
#define GType charptr
#define HtVectorGType HtVector_charptr
#include "HtVectorGenericCode.h"



// **************************************************
// *************** misc functions *******************
// **************************************************

// return a temporary string that merges a name and a number
char *
label_str (const char *s, int n)
{
  static char buff[1000];
  sprintf (buff, "%s%d", s, n);
  return buff;
}

// display n bits of value v
void
show_bits (int v, int n /*=16*/ )
{
  int i;
  if (n > 0)
  {
    for (i = 0; i < n; i++)
    {
      printf ("%c", (v & (1 << (n - i - 1)) ? '1' : '0'));
    }
  }
  else
  {
    n = -n;
    for (i = 0; i < n; i++)
    {
      printf ("%c", (v & (1 << (i)) ? '1' : '0'));
    }
  }
}



// duplicate an array of unsigned int's
unsigned int *
duplicate (unsigned int *v, int n)
{
  unsigned int *res = new unsigned int[n];
  CHECK_MEM (res);
  memcpy ((void *) res, (void *) v, n * sizeof (unsigned int));
  return (res);
}

// quick sort compare function (for unsigned int's)
int
qsort_uint_cmp (const void *a, const void *b)
{
//      printf("%12u %12u",*((unsigned int *)a),*((unsigned int *)b));
  if ((*((unsigned int *) a)) > (*((unsigned int *) b)))
    return 1;
  else if ((*((unsigned int *) a)) < (*((unsigned int *) b)))
    return -1;
  else
    return 0;
//      return 
//    (*((unsigned int *)a)) -
//    (*((unsigned int *)b))   ;
}

// quick sort an array of unsigned int's
void
qsort_uint (unsigned int *v, int n)
{
  qsort ((void *) v, (unsigned int) n, sizeof (unsigned int),
         &qsort_uint_cmp);
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
log2 (unsigned int v)
{
  int res;
  for (res = -1; v; res++)
  {
    v >>= 1;
  }
  return (res);
}




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
// table_size = 2^nlev * NBITS_NBITS_VAL
// coded_size = n * (nlev + SUM_interval_bit_sizes / 2^nlev )
//
// example1: flat probability distribution :
// SUM_interval_bit_sizes = 2^nlev * log2( 2^nbits / 2^nlev) = 2^nlev * ( nbits - nlev )
// => coded_size = n * ( nlev + nbits - nlev ) = n*nbits !!
// => coded_size is the same as if we used no compression
//    this is normal, because it is not possible to compress random data
//
// example2: probability all focused in first interval except for one entry
// SUM_interval_bit_sizes  = 1 + nbits
// the computations above are not valid because of integer roundofs
// => coded_size would actually be = n *  1 + nbits 
// (but the code needs a few cleanups to obtain this value) 
//
class VlengthCoder
{
  int nbits;                    // min number of bits to code all entries
  int nlev;                     // split proba into 2^nlev parts
  int nintervals;               // number of intervals

  int *intervals;
  unsigned int *intervalsizes;  // speedup
  unsigned int *lboundaries;    // speedup
    BitStream & bs;

//      inline unsigned int intervalsize(int i)
//    {
//        unsigned int res=((intervals[i] > 0 ? pow2(intervals[i]-1) : 0));
//        if(intervalsizes[i]!=res){errr("intervalsizes");}
//        return res;
//    }
  inline unsigned int intervalsize0 (int i)
  {
    return ((intervals[i] > 0 ? pow2 (intervals[i] - 1) : 0));
  }

public:
  int verbose;

  // find interval where value v resides
  // fast version, this one recursively splits initial interval
  inline int find_interval2 (const unsigned int v, unsigned int &lboundary)
  {
    int i0 = 0;
    int i1 = nintervals;
    int i;
    for (;;)
    {
      if (i1 == i0 + 1)
      {
        break;
      }
      i = (i0 + i1) >> 1;
      lboundary = lboundaries[i];
//        if(verbose)printf("considering i0:%3d i1:%3d : i:%3d  v:%12u lboundary:%12u (%12u - %12u)\n",i0,i1,i,v,lboundary,lboundaries[i0],lboundaries[i1]);
      if (v < lboundary)
      {
        i1 = i;
        continue;
      }
      else
      {
        i0 = i;
        continue;
      }

    }

    lboundary = lboundaries[i0];
//    i=i0;
//      unsigned int sboundary=lboundary+intervalsizes[i];
//      if(!( (lboundary!=sboundary && v>=lboundary && v<sboundary) || 
//          (lboundary==sboundary && v==lboundary)                   ))
//      {
//          printf("interval fd:i0:%3d i1:%3d : i:%3d  v:%12u lboundary:%12u (%12u - %12u)\n",i0,i1,i,v,lboundary,lboundaries[i0],lboundaries[i1]);
//          errr("bad interval");
//      }
    return i0;
  }

  // find interval where value v resides
  // slow version, this tries every interval
  inline int find_interval (const unsigned int v, unsigned int &lboundary)
  {
    // SPEED CRITICAL SECTION
    int i;
    unsigned int sboundary = 0;
    lboundary = 0;
    for (i = 0; i < nintervals - 1; i++)
    {
//        if(i>=nintervals){errr("code argh!");}
      sboundary = lboundary + intervalsizes[i];
//          printf("nintervals:%3d i:%3d : %12u ...  %12u  : %12u\n",nintervals,i,lboundary,sboundary,v);
      if ((lboundary != sboundary && v >= lboundary && v < sboundary) ||
          (lboundary == sboundary && v == lboundary))
      {
        break;
      }
      lboundary = sboundary;
    }

    return i;
  }

  // compress and insert a value into the bitstream
  inline void code (unsigned int v)
  {
    unsigned int lboundary = 0;
    // SPEED CRITICAL SECTION
    int i;
//    i=find_interval(v,lboundary);
    i = find_interval2 (v, lboundary);
    // were in the i'th interval;
    bs.put_uint (i, nlev, "int");       // store interval
    const int bitsremaining = (intervals[i] > 0 ? intervals[i] - 1 : 0);
//    if(verbose>1)printf("v:%6d interval:%2d (%5d - %5d) bitsremaining:%2d ",v,i,lboundary,sboundary,bitsremaining);
    v -= lboundary;
//    if(verbose>1)printf("remain:%6d  totalbits:%2d\n",v,bitsremaining+nlev);
    bs.put_uint (v, bitsremaining, "rem");
  }
  // get and uncompress  a value from  the bitstream
  inline unsigned int get ()
  {
    // SPEED CRITICAL SECTION
    int i = bs.get_uint (nlev, "int");  // get interval
//    if(verbose>1)printf("get:interval:%2d ",i);
    const int bitsremaining = (intervals[i] > 0 ? intervals[i] - 1 : 0);
//    if(verbose>1)printf("bitsremain:%2d ",bitsremaining);
    unsigned int v = bs.get_uint (bitsremaining, "rem");
//    if(verbose>1)printf("v0:%3d ",v);
//    unsigned int lboundary=0;
    v += lboundaries[i];
//  for(int j=0;j<i;j++){lboundary+=intervalsizes[j];}
//    v+=lboundary;
//    if(verbose>1)printf("lboundary:%5d v:%5d \n",lboundaries[i],v);
    return (v);
  }


  //  insert the packed probability distrbution into the bitstream
  void code_begin ();
  //  get the packed probability distrbution from the bitstream
  void get_begin ();

  void make_lboundaries ();

  VlengthCoder (BitStream & nbs, int nverbose = 0);

  ~VlengthCoder ()
  {
    delete[]lboundaries;
    delete[]intervals;
    delete[]intervalsizes;
  }

  // create VlengthCoder and its probability distrbution from an array of values
  VlengthCoder (unsigned int *vals, int n, BitStream & nbs, int nverbose = 0);
};

void
VlengthCoder::code_begin ()
{
  int i;
  bs.add_tag ("VlengthCoder:Header");
  bs.put_uint (nbits, NBITS_NBITS_VAL, "nbits");
  bs.put_uint (nlev, 5, "nlev");
  for (i = 0; i < nintervals; i++)
  {
    bs.put_uint (intervals[i], NBITS_NBITS_VAL, label_str ("interval", i));
  }
}

void
VlengthCoder::get_begin ()
{
  int i;
  nbits = bs.get_uint (NBITS_NBITS_VAL, "nbits");
  if (verbose > 1)
    printf ("get_begin nbits:%d\n", nbits);
  nlev = bs.get_uint (5, "nlev");
  if (verbose > 1)
    printf ("get_begin nlev:%d\n", nlev);
  nintervals = pow2 (nlev);

  intervals = new int[nintervals];
  CHECK_MEM (intervals);
  intervalsizes = new unsigned int[nintervals];
  CHECK_MEM (intervalsizes);
  lboundaries = new unsigned int[nintervals + 1];
  CHECK_MEM (lboundaries);

  for (i = 0; i < nintervals; i++)
  {
    intervals[i] = bs.get_uint (NBITS_NBITS_VAL, label_str ("interval", i));
    intervalsizes[i] = intervalsize0 (i);
    if (verbose > 1)
      printf ("get_begin intervals:%2d:%2d\n", i, intervals[i]);
  }
  make_lboundaries ();
}

void
VlengthCoder::make_lboundaries ()
{
  unsigned int lboundary = 0;
  for (int j = 0; j <= nintervals; j++)
  {
    lboundaries[j] = lboundary;
    if (j < nintervals)
    {
      lboundary += intervalsizes[j];
    }
  }
}

VlengthCoder::VlengthCoder (BitStream & nbs, int nverbose /*=0*/ ):
bs (nbs)
{
  verbose = nverbose;
  nbits = 0;
  nlev = 0;
  nintervals = 0;
  intervals = NULL;
}

int debug_test_nlev = -1;

VlengthCoder::VlengthCoder (unsigned int *vals, int n, BitStream & nbs,
                            int nverbose /*=0*/ ):
bs (nbs)
{
  verbose = nverbose;
  unsigned int *sorted = duplicate (vals, n);
  qsort_uint (sorted, n);

  nbits = num_bits (HtMaxMin::max_v (vals, n));

  // **** heuristics to determine best nlev
  // force table size to be less than 1/10 of the maximum coded size
  nlev = num_bits ((n * nbits) / (10 * NBITS_NBITS_VAL));
  // sanity
  if (nlev >= nbits)
  {
    nlev = nbits - 1;
  }
  // nlev at least 1
  if (nlev < 1)
  {
    nlev = 1;
  }

  if (debug_test_nlev >= 0)
  {
    nlev = debug_test_nlev;
  }
  nintervals = pow2 (nlev);
  int i;

  intervals = new int[nintervals];
  CHECK_MEM (intervals);
  intervalsizes = new unsigned int[nintervals];
  CHECK_MEM (intervalsizes);
  lboundaries = new unsigned int[nintervals + 1];
  CHECK_MEM (lboundaries);

  if (verbose > 1)
    printf ("nbits:%d nlev:%d nintervals:%d \n", nbits, nlev, nintervals);

  if (verbose > 10)
  {
    printf ("vals;\n");
    for (i = 0; i < n; i++)
    {
      printf ("%12u  ", vals[i]);
    }
    printf ("\nsorted:\n");
    for (i = 0; i < n; i++)
    {
      printf ("%12u  ", sorted[i]);
    }
    printf ("\n");
  }

  // find split boundaires
  unsigned int lboundary = 0;
  unsigned int boundary;
  for (i = 0; i < nintervals - 1; i++)
  {
    boundary = sorted[(n * (i + 1)) / nintervals];
    intervals[i] = 1 + log2 (boundary - lboundary);
    intervalsizes[i] = intervalsize0 (i);
    if (0 || verbose > 1)
      printf
        ("intnum%02d  begin:%5u end:%5u len:%5u (code:%2d)  real upper boundary: real:%5u\n",
         i, lboundary, intervalsizes[i] + lboundary, intervalsizes[i],
         intervals[i], boundary);
    lboundary += intervalsizes[i];
  }
  boundary = sorted[n - 1];
  intervals[i] = 1 + log2 (boundary - lboundary) + 1;
  intervalsizes[i] = intervalsize0 (i);
  if (0 || verbose > 1)
    printf
      ("intnum%02d  begin:%5u end:%5u len:%5u (code:%2d)  real upper boundary: real:%5u\n",
       i, lboundary, intervalsizes[i] + lboundary, intervalsizes[i],
       intervals[i], boundary);
  if (0 || verbose > 1)
    printf ("\n");

  make_lboundaries ();

  int SUM_interval_bit_sizes = 0;
  for (i = 0; i < nintervals; i++)
  {
    SUM_interval_bit_sizes += intervals[i];
  }
  if (verbose)
    printf ("SUM_interval_bit_sizes:%d\n", SUM_interval_bit_sizes);
  delete[]sorted;
}


// **************************************************
// *************** BitStream  ***********************
// **************************************************

void
BitStream::put_zone (byte * vals, int n, const char *tag)
{
  add_tag (tag);
  for (int i = 0; i < (n + 7) / 8; i++)
  {
    put_uint (vals[i], TMin (8, n - 8 * i), NULL);
  }
}

void
BitStream::get_zone (byte * vals, int n, const char *tag)
{
  check_tag (tag);
  for (int i = 0; i < (n + 7) / 8; i++)
  {
    vals[i] = get_uint (TMin (8, n - 8 * i));
  }
}

void
BitStream::put_uint (unsigned int v, int n, const char *tag /*="NOTAG"*/ )
{
  // SPEED CRITICAL SECTION
  if (freezeon)
  {
    bitpos += n;
    return;
  }
  add_tag (tag);

  if (!n)
  {
    return;
  }

  // 1)
  int bpos0 = bitpos & 0x07;
//      printf("bpos0:%3d bitpos:%5d:%5d  n:%4d  val:%x\n",bpos0,bitpos,buff.size()*8,n,v);
  if (bpos0 + n < 8)
  {
//      printf("simple case:");
//    ::show_bits(v,n);
//    printf("\n");
    // simplest case it all fits
    buff.back () |= v << bpos0;
    bitpos += n;
    if (!(bitpos & 0x07))
    {
      buff.push_back (0);
    }                           // new byte
    return;
  }
  else
  {
    const int ncentral = ((bpos0 + n) >> 3) - 1;
    // put first
    buff.back () |= ((v & 0xff) << bpos0) & 0xff;
    const int nbitsinfirstbyte = 8 - bpos0;

//    printf("normal case :(%x:%x)",((v & 0xff)<<bpos0) & 0xff,buff.back());
//    ::show_bits(((v & 0xff)<<bpos0) & 0xff,-8);
//    printf(" ");


    v >>= nbitsinfirstbyte;
//    printf(" (v:%x)",v);
    // put central
    for (int i = ncentral; i; i--)
    {
      buff.push_back (0);
      buff.back () = v & 0xff;
//        ::show_bits(v & 0xff,-8);
//        printf(" ");
      v >>= 8;
    }
    // put last
    const int nbitsremaining = n - ((ncentral << 3) + nbitsinfirstbyte);
    if (nbitsremaining)
    {
      buff.push_back (0);
      buff.back () = v & (pow2 (nbitsremaining + 1) - 1);

//        printf(" (v:%x:%x)",v &  (pow2(nbitsremaining+1)-1),buff.back());
//        ::show_bits(v &  (pow2(nbitsremaining+1)-1),-nbitsremaining);
//        printf("\n");
    }
    if (!(nbitsremaining & 0x07))
    {
      buff.push_back (0);
    }
    bitpos += n;
//    printf("nbitsinfirstbyte:%d ncentral:%d  nbitsremaining:%d\n",nbitsinfirstbyte,ncentral,nbitsremaining);

  }
//      printf("cuurent put order:");
//      for(i=0;i<n;i++)
//      {
//    printf("%c",((v0& pow2(i) ? '1':'0')));
//      }
//      printf("\n");
}




unsigned int
BitStream::get_uint (int n, const char *tag /*=NULL*/ )
{
  // SPEED CRITICAL SECTION
  if (check_tag (tag) == NOTOK)
  {
    errr ("BitStream::get(int) check_tag failed");
  }
  if (!n)
  {
    return 0;
  }

  unsigned int res = 0;

  // 1)
  int bpos0 = bitpos & 0x07;

//      printf("bpos0:%3d bitpos:%5d  n:%4d %s\n",bpos0,bitpos,n,tag);
//      printf("input:\n");
//      for(int j=0;j<(bpos0+n+7)/8;j++){printf("%x",buff[bitpos/8+j]);}
//      printf("\n");

  if (bpos0 + n < 8)
  {
    // simplest case it all fits
    res = (buff[bitpos >> 3] >> bpos0) & (pow2 (n) - 1);
    bitpos += n;
//        printf("simple case:res:%x\n",res);
    return res;
  }
  else
  {
    int bytepos = bitpos >> 3;
    const int ncentral = ((bpos0 + n) >> 3) - 1;
    // put first
    res = (buff[bytepos] >> bpos0) & 0xff;
//          printf("normal case:res0:%x\n",res);

    const int nbitsinfirstbyte = 8 - bpos0;

    bytepos++;
    // put central
    if (ncentral)
    {
      unsigned int v = 0;
      for (int i = ncentral - 1; i >= 0; i--)
      {
        v |= buff[bytepos + i] & 0xff;
        if (i)
          v <<= 8;
//        printf("       resC%d:v:%x\n",i,v);
      }
      bytepos += ncentral;
      res |= v << nbitsinfirstbyte;
//          printf("       :resC:%x\n",res);
    }
    // put last
    const int nbitsremaining = n - ((ncentral << 3) + nbitsinfirstbyte);
    if (nbitsremaining)
    {
      res |=
        ((unsigned int) (buff[bytepos] & (pow2 (nbitsremaining) - 1))) <<
        (nbitsinfirstbyte + ((bytepos - (bitpos >> 3) - 1) << 3));
//          printf("       :resR:%x  buff[%d]:%x  %d\n",res,bytepos,buff[bytepos],
//           (nbitsinfirstbyte +((bytepos-(bitpos>>3)-1)<<3)));
    }

    bitpos += n;
//        printf("nbitsinfirstbyte:%d ncentral:%d  nbitsremaining:%d\n",nbitsinfirstbyte,ncentral,nbitsremaining);
    return res;
  }
}

#ifdef NOTDEF
unsigned int
BitStream::get (int n, const char *tag /*=NULL*/ )
{
  if (check_tag (tag) == NOTOK)
  {
    errr ("BitStream::get(int) check_tag failed");
  }
  unsigned int res = 0;
  for (int i = 0; i < n; i++)
  {
    if (get ())
    {
      res |= pow2 (i);
    }
  }
  return (res);
}
#endif
void
BitStream::freeze ()
{
  freeze_stack.push_back (bitpos);
  freezeon = 1;
}

int
BitStream::unfreeze ()
{
  int size0 = bitpos;
  bitpos = freeze_stack.back ();
  freeze_stack.pop_back ();
  size0 -= bitpos;
  if (freeze_stack.size () == 0)
  {
    freezeon = 0;
  }
  return (size0);
}

void
BitStream::add_tag1 (const char *tag)
{
  if (!use_tags)
  {
    return;
  }
  if (freezeon)
  {
    return;
  }
  if (!tag)
  {
    return;
  }
  tags.push_back (strdup (tag));
  tagpos.push_back (bitpos);
}

int
BitStream::check_tag1 (const char *tag, int pos /*=-1*/ )
{
  if (!use_tags)
  {
    return OK;
  }
  if (!tag)
  {
    return OK;
  }
  int found = -1;
  int ok = 0;
  if (pos == -1)
  {
    pos = bitpos;
  }
  for (int i = 0; i < tags.size (); i++)
  {
    if (!strcmp (tags[i], tag))
    {
      found = tagpos[i];
      if (tagpos[i] == pos)
      {
        ok = 1;
        break;
      }
    }
  }
  if (!ok)
  {
    show ();
    if (found >= 0)
    {
      printf
        ("ERROR:BitStream:bitpos:%4d:check_tag: found tag %s at %d expected it at %d\n",
         bitpos, tag, found, pos);
    }
    else
    {
      printf
        ("ERROR:BitStream:bitpos:%4d:check_tag:  tag %s not found, expected it at %d\n",
         bitpos, tag, pos);
    }
    return (NOTOK);
  }
  return (OK);
}

int
BitStream::find_tag (const char *tag)
{
  int i;
  for (i = 0; i < tags.size () && strcmp (tag, tags[i]); i++);
  if (i == tags.size ())
  {
    return -1;
  }
  else
  {
    return i;
  }
}

int
BitStream::find_tag (int pos, int posaftertag /*=1*/ )
{
  int i;
  for (i = 0; i < tags.size () && tagpos[i] < pos; i++);
  if (i == tags.size ())
  {
    return -1;
  }
  if (!posaftertag)
  {
    return i;
  }
  for (; tagpos[i] > pos && i >= 0; i--);
  return (i);
}

void
BitStream::show_bits (int a, int n)
{
  for (int b = a; b < a + n; b++)
  {
    printf ("%c", (buff[b / 8] & (1 << (b % 8)) ? '1' : '0'));
  }
}

void
BitStream::show (int a /*=0*/ , int n /*=-1*/ )
{
  int all = (n < 0 ? 1 : 0);
  if (n < 0)
  {
    n = bitpos - a;
  }
  int i;

  if (all)
  {
    printf ("BitStream::Show: ntags:%d size:%4d buffsize:%6d ::: ",
            tags.size (), size (), buffsize ());
//        for(i=0;i<tags.size();i++){printf("tag:%d:%s:pos:%d\n",i,tags[i],tagpos[i]);}
  }

  int t = find_tag (a, 0);
  if (t < 0)
  {
    show_bits (a, n);
    return;
  }
  for (i = a; i < a + n; i++)
  {
    for (; t < tags.size () && tagpos[t] < i + 1; t++)
    {
      printf ("# %s:%03d:%03d #", tags[t], tagpos[t], n);
    }
    show_bits (i, 1);
  }
  if (all)
  {
    printf ("\n");
  }

}

byte *
BitStream::get_data ()
{
  byte *res = (byte *) malloc (buff.size ());
  CHECK_MEM (res);
  for (int i = 0; i < buff.size (); i++)
  {
    res[i] = buff[i];
  }
  return (res);
}

void
BitStream::set_data (const byte * nbuff, int nbits)
{
  if (buff.size () != 1 || bitpos != 0)
  {
    printf ("BitStream:set_data: size:%d bitpos:%d\n", buff.size (), bitpos);
    errr ("BitStream::set_data: valid only if BitStream is empty");
  }
  buff[0] = nbuff[0];
  for (int i = 1; i < (nbits + 7) / 8; i++)
  {
    buff.push_back (nbuff[i]);
  }
  bitpos = nbits;
}



// **************************************************
// *************** Compressor ***********************
// **************************************************


void
Compressor::put_uint_vl (unsigned int v, int maxn,
                         const char *tag /*="NOTAG"*/ )
{
  int nbits = num_bits (v);
  put_uint (nbits, num_bits (maxn), tag);
  if (nbits)
  {
    put_uint (v, nbits, (char *) NULL);
  }
}

unsigned int
Compressor::get_uint_vl (int maxn, const char *tag /*=NULL*/ )
{
  int nbits = get_uint (num_bits (maxn), tag);
  if (!nbits)
  {
    return 0;
  }
  else
  {
    return (get_uint (nbits, (char *) NULL));
  }
}

int
Compressor::put_vals (unsigned int *vals, int n, const char *tag)
{
  int cpos = bitpos;
  add_tag (tag);
  if (n >= pow2 (NBITS_NVALS))
  {
    errr ("Compressor::put(uint *,nvals) : overflow: nvals>2^16");
  }
  put_uint_vl (n, NBITS_NVALS, "size");
  if (n == 0)
  {
    return NBITS_NVALS;
  }

  int sdecr = 2;
  int sfixed = 1;

  int nbits = num_bits (HtMaxMin::max_v (vals, n));
  if (verbose)
    printf ("*********************put_vals:n:%3d nbits:%3d\n", n, nbits);

  int i;
  if (verbose)
  {
    printf ("TTT:n:%3d nbits:%3d\n", n, nbits);
    for (i = 1; i < 7; i++)
    {
      debug_test_nlev = i;
      printf ("trying nlev:%3d\n", debug_test_nlev);
      freeze ();
      put_decr (vals, n);
      int fndsz = unfreeze ();
      printf ("TTT:nlev:%2d try size:%4d\n", i, fndsz);
    }
    debug_test_nlev = -1;
  }

  if (n > 15 && nbits > 3)
  {
    freeze ();
    put_decr (vals, n);
    sdecr = unfreeze ();

    freeze ();
    put_fixedbitl (vals, n);
    sfixed = unfreeze ();
  }

  if (verbose)
    printf ("put_vals:n:%3d sdecr:%6d sfixed:%6d rap:%f\n", n, sdecr, sfixed,
            sdecr / (float) sfixed);
  if (sdecr < sfixed)
  {
    if (verbose)
      printf ("put_vals: comptyp:0\n");
    put_uint (0, 2, "put_valsCompType");
    put_decr (vals, n);
  }
  else
  {
    if (verbose)
      printf ("put_vals: comptyp:1\n");
    put_uint (1, 2, "put_valsCompType");
    put_fixedbitl (vals, n);
  }

  if (verbose)
    printf ("------------------------------put_vals over\n");

  return (bitpos - cpos);
}

int
Compressor::get_vals (unsigned int **pres, const char *tag /*="BADTAG!"*/ )
{
  if (check_tag (tag) == NOTOK)
  {
    errr ("Compressor::get_vals(unsigned int): check_tag failed");
  }
  int n = get_uint_vl (NBITS_NVALS);
  if (verbose > 1)
    printf ("get_vals n:%d\n", n);
  if (!n)
  {
    *pres = NULL;
    return 0;
  }

  if (verbose)
    printf ("get_vals: n:%3d\n", n);
  unsigned int *res = new unsigned int[n];
  CHECK_MEM (res);


  int comptype = get_uint (2, "put_valsCompType");
  if (verbose)
    printf ("get_vals:comptype:%d\n", comptype);
  switch (comptype)
  {
  case 0:
    get_decr (res, n);
    break;
  case 1:
    get_fixedbitl (res, n);
    break;
  default:
    errr ("Compressor::get_vals invalid comptype");
    break;
  }
//      get_fixedbitl(res,n);
//      get_decr(res,n);

  *pres = res;
  return (n);
}


int
Compressor::put_fixedbitl (byte * vals, int n, const char *tag)
{
  int cpos = bitpos;
  int i, j;
  add_tag (tag);

  put_uint_vl (n, NBITS_NVALS, "size");
  if (n == 0)
  {
    return 0;
  }

  byte maxv = vals[0];
  for (i = 1; i < n; i++)
  {
    byte v = vals[i];
    if (v > maxv)
    {
      maxv = v;
    }
  }
  int nbits = num_bits (maxv);
  if (n >= pow2 (NBITS_NVALS))
  {
    errr ("Compressor::put_fixedbitl(byte *) : overflow: nvals>2^16");
  }
  put_uint (nbits, NBITS_NBITS_CHARVAL, "nbits");
  add_tag ("data");
  for (i = 0; i < n; i++)
  {
    byte v = vals[i];
    for (j = 0; j < nbits; j++)
    {
      put (v & pow2 (j));
    }
  }
  return (bitpos - cpos);
}

void
Compressor::put_fixedbitl (unsigned int *vals, int n)
{
  int nbits = num_bits (HtMaxMin::max_v (vals, n));

  put_uint_vl (nbits, NBITS_NBITS_VAL, "nbits");
  add_tag ("data");
  if (verbose)
    printf ("put_fixedbitl:nbits:%4d nvals:%6d\n", nbits, n);
  for (int i = 0; i < n; i++)
  {
    put_uint (vals[i], nbits, NULL);
  }
}

void
Compressor::get_fixedbitl (unsigned int *res, int n)
{
  int nbits = get_uint_vl (NBITS_NBITS_VAL);
  if (verbose)
    printf ("get_fixedbitl(uint):n%3d nbits:%2d\n", n, nbits);
  int i;
  for (i = 0; i < n; i++)
  {
    res[i] = get_uint (nbits);
  }
}

int
Compressor::get_fixedbitl (byte ** pres, const char *tag /*="BADTAG!"*/ )
{
  if (check_tag (tag) == NOTOK)
  {
    errr ("Compressor::get_fixedbitl(byte *): check_tag failed");
  }
  int n = get_uint_vl (NBITS_NVALS);
  if (!n)
  {
    *pres = NULL;
    return 0;
  }
  int nbits = get_uint (NBITS_NBITS_CHARVAL);
  if (verbose)
    printf ("get_fixedbitl(byte):n%3d nbits:%2d\n", n, nbits);
  int i;
  byte *res = new byte[n];
  CHECK_MEM (res);
  for (i = 0; i < n; i++)
  {
    res[i] = get_uint (nbits);
  }
  *pres = res;
  return (n);
}

void
Compressor::put_decr (unsigned int *vals, int n)
{
  VlengthCoder coder (vals, n, *this, verbose);
  coder.code_begin ();
  int i;
  for (i = 0; i < n; i++)
  {
    coder.code (vals[i]);
  }
}

void
Compressor::get_decr (unsigned int *res, int n)
{
  VlengthCoder coder (*this, verbose);
  coder.get_begin ();
  int i;
  for (i = 0; i < n; i++)
  {
    res[i] = coder.get ();
    if (verbose > 1)
    {
      printf ("get_decr:got:%8d\n", res[i]);
    }
  }
}
