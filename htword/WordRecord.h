//
// WordRecord.h
//
// WordRecord: Record for storing word information in the word database
//             Each word occurence is stored as a separate key/record pair.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordRecord.h,v 1.1 1999/09/30 15:56:46 loic Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

struct WordRecord
{
    unsigned int		anchor;

    void	Clear()
      {
	anchor = 0;
      }
};

/* And this is how we will compress this structure, for disk
   storage.  See HtPack.h  (If there's a portable method by
   which this format string does not have to be specified at
   all, it should be preferred.  For now, at least it is kept
   here, together with the actual struct declaration.)

   Since none of the values are non-zero, we want to use
   unsigned chars and unsigned short ints when possible. */

#define WORD_RECORD_COMPRESSED_FORMAT "u"

#endif


