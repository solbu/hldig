// WordKeyInfo.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <errno.h>

#include "WordKeyInfo.h"
#include "StringList.h"

#define WORDKEYFIELD_BITS_MAX 64

//
// WordKeyInfo implementation
//

WordKeyInfo::WordKeyInfo(const Configuration& config)
{
  nfields = -1;

  const String &keydesc = config["wordlist_wordkey_description"];

  if(!keydesc.empty()) {
    Set(keydesc);
  } else {
    fprintf(stderr, "WordKeyInfo::WordKeyInfo: didn't find key description in config\n");
  }
}

int
WordKeyInfo::Set(const String &desc)
{
  int ret = 0;
  StringList line(desc, "/");

  if(line.Count() > WORD_KEY_MAX_NFIELDS) {
    fprintf(stderr, "WordKeyInfo::Set: too many fields in %s, max is %d\n", (const char*)desc, WORD_KEY_MAX_NFIELDS);
    return EINVAL;
  }

  if(line.Count() <= 0) {
    fprintf(stderr, "WordKeyInfo::Set: no fields\n");
    return EINVAL;
  }

  int i;
  for(i = 0; i < line.Count(); i++) {
    char* field = line[i];
    WordKeyField& key_field = fields[i];
    //
    // Numerical field
    //
    StringList pair(field, "\t ");
	
    if(pair.Count() != 2) {
      fprintf(stderr, "WordKeyInfo::AddField: there must be exactly two strings separated by a white space (space or tab) in a field description (%s in key description %s)\n", field, (const char*)desc);
      return EINVAL;
    }

    key_field.bits = atoi(pair[1]);
    key_field.name = pair[0];
  }

  nfields = line.Count();

  return ret;
}

