//
// WordRecord.h
//
// $Id: WordRecord.h,v 1.8 1999/08/31 07:24:28 ghutchis Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

//
// Flags
// 
#define FLAG_TEXT 0
#define FLAG_CAPITAL 1
#define FLAG_TITLE 2
#define FLAG_HEADING 4
#define FLAG_KEYWORDS 8
#define FLAG_DESCRIPTION 16
#define FLAG_AUTHOR 32
#define FLAG_LINK_TEXT 64
#define FLAG_URL 128
// The remainder are undefined

struct WordRecord
{
    int		id;
    int		flags;
    int		anchor;
    int		location;

    void	Clear()
      {
	id = flags = anchor = location = 0;
      }
};

/* And this is how we will compress this structure, for disk
   storage.  See HtPack.h  (If there's a portable method by
   which this format string does not have to be specified at
   all, it should be preferred.  For now, at least it is kept
   here, together with the actual struct declaration.)

   Since none of the values are non-zero, we want to use
   unsigned chars and unsigned short ints when possible. */

#define WORD_RECORD_COMPRESSED_FORMAT "u4"

#endif


