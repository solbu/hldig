//
// WordList.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordList.cc,v 1.6.2.39 2000/09/21 04:25:35 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordList.h"
#include "WordDBCache.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

static int word_db_qcmp(WordContext* context, const WordDBCacheEntry *a, const WordDBCacheEntry *b)
{
  return WordKey::Compare(context, (const unsigned char*)a->key, a->key_size, (const unsigned char*)b->key, b->key_size);
}

// *****************************************************************************
void
WordList::BatchStart()
{
  if(caches) BatchEnd();

  Configuration& config = context->GetConfiguration();
  int cache_size = config.Value("wordlist_cache_size", 0);
  if(cache_size < 1 * 1024 * 1024) cache_size = 1 * 1024 * 1024;
  int cache_max = config.Value("wordlist_cache_max", 0);

  caches = new WordDBCaches(this, 50, cache_size, cache_max);
  caches->CacheCompare(word_db_qcmp);
}

// *****************************************************************************
void
WordList::BatchEnd()
{
  if(caches) {
    delete caches;
    caches = 0;
  }
}
