//
// WordRecord.h
//
// $Id: WordRecord.h,v 1.4 1999/03/21 15:19:27 hp Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

struct WordRecord
{
#ifndef NO_WORD_COUNT
	int		count;
#endif
	int		id;
	int		weight;
	int		anchor;
	int		location;

	void	Clear()
		{
		  id = weight = anchor = location = 0;
#ifndef NO_WORD_COUNT
		  count = 1;
#endif
		}
};

/* And this is how we will compress this structure, for disk
   storage.  See HtPack.h  (If there's a portable method by
   which this format string does not have to be specified at
   all, it should be preferred.  For now, at least it is kept
   here, together with the actual struct declaration.)

   Since none of the values are non-zero, we want to use
   unsigned chars and unsigned short ints when possible. */

#ifdef NO_WORD_COUNT
#define WORD_RECORD_COMPRESSED_FORMAT "u4"
#else
#define WORD_RECORD_COMPRESSED_FORMAT "u5"
#endif

#endif


