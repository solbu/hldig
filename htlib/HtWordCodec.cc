//
// Translation methods for HtWordCodec
//
// $Id: HtWordCodec.cc,v 1.2.2.1 1999/03/23 20:41:55 grdetil Exp $
//
//

#include "HtWordCodec.h"

// Do not use 0, so we can use "normal" string routines.
// Values 1..4 are used to describe how many bytes are used to
// keep the number.  Do not use other than control-characters,
// as the first character for internal encodings, so the user
// can use "international" characters (128 .. 255) for cute
// encodings to use across different configuration files and
// databases.
#define JOIN_CHAR 5
#define QUOTE_CHAR 6
#define FIRST_INTERNAL_SINGLECHAR 7
#define LAST_INTERNAL_SINGLECHAR 31


HtWordCodec::HtWordCodec()
{
  myFrom = 0;
  myTo = 0;
  myFromMatch = 0;
  myToMatch = 0;
}


HtWordCodec::~HtWordCodec()
{
  if (myFrom)
    delete myFrom;

  if (myTo)
    delete myTo;

  if (myFromMatch)
    delete myFromMatch;

  if (myToMatch)
    delete myToMatch;
}


// Straightforward filling of the encoding-lists.
HtWordCodec::HtWordCodec(StringList *from, StringList *to, char joiner)
{
  myFromMatch = new StringMatch;
  myToMatch = new StringMatch;

  myTo = to;
  myFrom = from;

  String to_pattern(myTo->Join(joiner));

  // After being initialized with Join, the strings are not
  // null-terminated, but that is done through "operator char*".
  myToMatch->Pattern(to_pattern, joiner);

  String from_pattern(myFrom->Join(joiner));
  myFromMatch->Pattern(from_pattern, joiner);

}


// This constructor is the most complicated function in this class.
// It handles consistency checking for the supplied code-lists.

// Cleanups for anything except myTo, myFrom, myToMatch is
// necessary.  The member myFromMatch is used as a sanity check
// for member functions to see that the constructor was
// successful in case the programmer forgets to check errmsg.
HtWordCodec::HtWordCodec(StringList &requested_encodings,
                         StringList &frequent_substrings,
                         String &errmsg)
{
  if ((requested_encodings.Count() % 2) != 0)
  {
    errmsg =
      "Expected pairs, got odd number of strings";

    return;
  }

  myFrom = new StringList;
  myTo = new StringList;

  // Go through requested_encodings and fill myTo and myFrom.
  // Check that the "to" strings look remotely sane regarding
  // reserved characters.

  // Iteration temporaries.
  String *from;
  String *to;

  int n_of_pairs = requested_encodings.Count() / 2;

  requested_encodings.Start_Get();
  while ((from = (String *) requested_encodings.Get_Next()) != NULL)
  {
    // Sanity check: Reserve empty strings as we cannot do
    // anything sane with them.

    int templen = from->length();
    if (templen == 0)
    {
      errmsg = "Empty strings are not allowed";
      return;
    }

    myFrom->Add(from);

    // This must be non-null since we checked "oddness" above.
    to = (String *) requested_encodings.Get_Next();

    templen = to->length();
    if (templen == 0)
    {
      errmsg = "Empty strings are not allowed";
      return;
    }

    // We just have to check that there's no JOIN_CHAR in the
    // string.  Since no "to" is allowed to be part of any other
    // "to", there will be no ambiguity, even if one would
    // contain a QUOTE_CHAR (which is documented as invalid anyway).
    if (strchr(from->get(), JOIN_CHAR) != NULL)
    {
      errmsg =
        form("(\"%s\" =>) \"%s\" contains a reserved character (number %d)",
             from->get(), to->get(), int(JOIN_CHAR));
      return;
    }

    // Loop over the other "to"-strings and check that this
    // string is not a  substring of any other "to", or vice versa.
    // Return in error if it is so.
    int i;
    int count = myTo->Count();
    for (i = 0; i < count; i++)
    {
      String *ith = (String *) myTo->Nth(i);

      // Just check if the shorter string is part of the
      // longer string.
      if (to->length() < ith->length()
          ? ith->indexOf(to->get()) != -1
          : to->indexOf(ith->get()) != -1)
      {
        errmsg =
          form("\"%s\" => \"%s\" collides with (\"%s\" => \"%s\")",
               from, to, (*myFrom)[i], ith->get());

        return;
      }
    }

    // All ok, just add this one.
    myTo->Add(to);
  }

  // Check that none of the "to"-strings is a substring of any
  // of the "from" strings, since that's hard to support and
  // most probably is a user mistake anyway.

  StringMatch req_tos;
  String req_to_pattern(myTo->Join(JOIN_CHAR));
  int which, length;

  // The StringMatch functions want the strings
  // zero-terminated, which is done through "operator char*".
  req_tos.Pattern(req_to_pattern, JOIN_CHAR);

  // Check the requested encodings.
  if (n_of_pairs != 0)
  {
    int i;
    for (i = 0; i < n_of_pairs; i++)
    {
      from = (String *) myFrom->Nth(i);
      if (req_tos.FindFirst(from->get(), which, length) != -1)
      {
        if (i != which)
        {
          errmsg =
            form("(\"%s\" => \"%s\") overlaps (\"%s\" => \"%s\")",
                 (*myFrom)[which], (*myTo)[which],
                 from->get(), (*myTo)[i]);
        }
        else
        {
          errmsg =
            form("Overlap in (\"%s\" => \"%s\")",
                 from->get(), (*myTo)[i]);
        }

        return;
      }
    }
  }

  if (frequent_substrings.Count() != 0)
  {
    // Make a temporary search-pattern of the requested
    // from-strings.

    StringMatch req_froms;
    String req_from_pattern(myFrom->Join(JOIN_CHAR));

    req_froms.Pattern(req_from_pattern, JOIN_CHAR);

    // Continue filling "to" and "from" from frequent_substrings and
    // internal encodings.  If a frequent_substring is found in the
    // requested from-strings, it is ignored, but the internal
    // encoding is still ticked up, so that changes in
    // requested_encodings (e.g. url_part_aliases) do not change
    // an existing database (e.g. containing common_url_parts).

    int internal_encoding_no = 0;

    String *common_part;
    frequent_substrings.Start_Get();
    String to;

    for (;
         (common_part = (String *) frequent_substrings.Get_Next()) != NULL;
         internal_encoding_no++)
    {
      int templen = common_part->length();
      if (templen == 0)
      {
        errmsg = "Empty strings are not allowed";
        return;
      }

      // Is a "From" string in it, or is a "To" string in it?
      //  Note that checking if there are *any* requested
      // encodings (n_of_pairs) is not just an "optimization";
      // it is necessary since StringMatch will return 0 (not
      // -1) if the pattern is empty (FIXME: changing that
      // breaks something else in another part of ht://Dig).

      if (n_of_pairs
          && (req_froms.FindFirst(common_part->get()) != -1
              || req_tos.FindFirst(common_part->get()) != -1))
        continue;

      to = 0;                   // Clear previous run.

      // Dream up an encoding without zeroes.
      // Use FIRST_INTERNAL_SINGLECHAR .. LAST_INTERNAL_SINGLECHAR
      // for the first encodings, as much as possible.

      long int number_to_store =
        internal_encoding_no + FIRST_INTERNAL_SINGLECHAR;

      if (number_to_store <= LAST_INTERNAL_SINGLECHAR)
      {
        to << char(number_to_store);
      }
      else
      {
        // Use <number-of-bytes-in-length>
        // <number-as-nonzero-bytes> to code the rest.
        //  Note that we assume eight-bit chars here, which
        // should be ok for all systems you run htdig on.
        // At least it helps clarity here.

        number_to_store -= LAST_INTERNAL_SINGLECHAR;

        // Make sure highest bit in every byte is "1" by
        // inserting one there.
        char to_store[sizeof(number_to_store)+1];
        int j = 1;

        while (number_to_store > 0x7f)
        {
          number_to_store = ((number_to_store & ~0x7f) << 1)
            | 0x80 | (number_to_store & 0x7f);

          to_store[j++] = char(number_to_store);
          number_to_store >>= 8;
        }

        // Finally, store the highest byte.  It too shall have
        // the highest bit set.  This is the easiest way to
        // adjust it not to be QUOTE_CHAR.
        to_store[0] = j;
        to_store[j] = char(number_to_store | 0x80);

        to.append(to_store, j+1);
      }

      // Add to replacement pairs.
      myFrom->Add(common_part);
      myTo->Add(&to);
    }
  }

  // Now, add the quoted "to":s to the "to"-list, with the unquoted
  // "to":s to the "from"-list.  This way we do not have to
  //  check for quoting separately.  Like this:
  // From  To
  // foo : !
  // bar : >
  // baz : $
  // !   : \!
  // >   : \>
  // $   : \$
  //
  // Since we checked that none of the "To":s are in a "From" we
  // can do this.

  myTo->Start_Get();
  int to_count = myTo->Count();
  String *current;
  String temp;

  int i;
  for (i = 0; i < to_count; i++)
  {
    // It works to append *and* iterate through a
    // StringList, despite not having an iterator class.
    current = (String *) myTo->Nth(i);

    myFrom->Add(current);

    temp = 0; // Reset any previous round.
    temp.append(char(QUOTE_CHAR));
    temp.append(*current);

    myTo->Add(&temp);
  }

  myFromMatch = new StringMatch;
  myToMatch = new StringMatch;

  String to_pattern(myTo->Join(JOIN_CHAR));
  String from_pattern(myFrom->Join(JOIN_CHAR));

  // StringMatch class has unchecked limits, better check them.
  // The length of each string in the pattern an the upper limit
  // of the needs.
  if (to_pattern.length() - (myTo->Count() - 1) > 0xffff
      || from_pattern.length() - (myFrom->Count() - 1) > 0xffff)
  {
    errmsg = "Limit reached; use fewer encodings";
    return;
  }

  myToMatch->Pattern(to_pattern, JOIN_CHAR);
  myFromMatch->Pattern(from_pattern, JOIN_CHAR);

  errmsg = 0;
}


// We only need one "coding" function, since quoting and unquoting is
// handled through the to- and from-lists.
String
HtWordCodec::code(const String &orig_string, StringMatch &match,
                  StringList &replacements) const
{
  String retval;
  String tempinput;
  int offset, which, length;
  char *orig;

  // Get a null-terminated string, usable for FindFirst to look at.
  orig = orig_string.get();

  // Sanity check.  If bad use, just return empty strings.
  if (myFromMatch == NULL)
  {
    return retval;
  }

  // Need to check if "replacements" is empty; that is, if no
  // transformations should be done.  FindFirst() does not return
  // -1 in this case, it returns 0.
  if (replacements.Count() == 0)
    return orig_string;

  // Find the encodings and replace them.
  while ((offset = match.FindFirst(orig, which, length)) != -1)
  {
    // Append the previous part that was not part of a code.
    retval.append(orig, offset);

    // Replace with the original string.
    retval.append(replacements[which]);

    orig += offset + length;
  }

  // Add the final non-matched part.
  retval.append(orig);

  return retval;
}


// The assymetry is caused by swapping both the matching and
// replacement lists.
String
HtWordCodec::decode(const String &orig) const
{
  return code(orig, *myToMatch, *myFrom);
}


String
HtWordCodec::encode(const String &orig) const
{
  return code(orig, *myFromMatch, *myTo);
}

// End of HtWordCodec.cc
