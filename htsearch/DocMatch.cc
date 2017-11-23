// DocMatch.cc
//
// DocMatch: Data object only. Contains information related to a given
//           document that was matched by a search. For instance, the
//           score of the document for this search.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: DocMatch.cc,v 1.8 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "DocMatch.h"
#include "HtConfiguration.h"
#include "HtWordReference.h"

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

//*******************************************************************************
// DocMatch::DocMatch()
//


//*******************************************************************************
// DocMatch::~DocMatch()
//
DocMatch::~DocMatch ()
{
}

//
// merge with another match
// sets anchor to the lower value
// merges location lists
//
void
DocMatch::Merge (const DocMatch & match)
{
  if (match.anchor < anchor)
  {
    anchor = match.anchor;
  }
  AddLocations (match.GetLocations ());
}

//
// adds locations to an existing list
// avoiding duplicates, in location order
//
void
DocMatch::AddLocations (const List * locs)
{
  List *merge = new List;
  ListCursor c;

  locations->Start_Get ();
  locs->Start_Get (c);
  Location *a = (Location *) locations->Get_Next ();
  Location *b = (Location *) locs->Get_Next (c);
  while (a && b)
  {
    if (a->from < b->from)
    {
      merge->Add (a);
      a = (Location *) locations->Get_Next ();
    }
    else if (a->from > b->from)
    {
      merge->Add (new Location (*b));
      b = (Location *) locs->Get_Next (c);
    }
    else                        // (a->from == b->from)
    {
      if (a->to < b->to)
      {
        merge->Add (new Location (*a));
        merge->Add (new Location (*b));
      }
      else if (a->to > b->to)
      {
        merge->Add (new Location (*b));
        merge->Add (new Location (*a));
      }
      else                      // (a->to == b->to)
      {
        merge->Add (new Location (a->from,
                                  a->to, a->flags, a->weight + b->weight));
      }
      a = (Location *) locations->Get_Next ();
      b = (Location *) locs->Get_Next (c);
    }
  }
  while (a)
  {
    merge->Add (a);
    a = (Location *) locations->Get_Next ();
  }
  while (b)
  {
    merge->Add (new Location (*b));
    b = (Location *) locs->Get_Next (c);
  }
  locations->Release ();
  delete locations;
  locations = merge;
}

//
// set the location list
//
void
DocMatch::SetLocations (List * locs)
{
  delete locations;
  locations = locs;
}

//
// copy constructor, copies locations
//
DocMatch::DocMatch (const DocMatch & other)
{
  score = -1.0;
  //score = other.score;
  id = other.id;
  anchor = other.anchor;
  locations = new List;
  AddLocations (other.GetLocations ());
}

//
// set weight of all locations
//
void
DocMatch::SetWeight (double weight)
{
  locations->Start_Get ();
  for (int i = 0; i < locations->Count (); i++)
  {
    Location *loc = (Location *) locations->Get_Next ();
    loc->weight = weight;
  }
}

//
// debug dump
//
void
DocMatch::Dump ()
{
  cerr << "DocMatch id: " << id << " {" << endl;
  locations->Start_Get ();
  for (int i = 0; i < locations->Count (); i++)
  {
    Location *loc = (Location *) locations->Get_Next ();
    cerr << "location [" << loc->from;
    cerr << ", " << loc->to << "] ";
    cerr << "weight " << loc->weight;
    cerr << " flags " << loc->flags;
    cerr << endl;
  }
  cerr << "score: " << GetScore () << endl << "}" << endl;
}

double
DocMatch::GetScore ()
{
  HtConfiguration *config = HtConfiguration::config ();
  static double text_factor = config->Double ("text_factor", 1);
  static double caps_factor = config->Double ("caps_factor", 1);
  static double title_factor = config->Double ("title_factor", 1);
  static double heading_factor = config->Double ("heading_factor", 1);
  static double keywords_factor = config->Double ("keywords_factor", 1);
  static double meta_desc_factor =
    config->Double ("meta_description_factor", 1);
  static double author_factor = config->Double ("author_factor", 1);
  static double description_factor = config->Double ("description_factor", 1);
  static double url_text_factor = config->Double ("url_text_factor", 1);

  if (score == -1.0)
  {
    score = 0.0;

    double locresult = 0.0;
    ListCursor c;
    locations->Start_Get (c);
    Location *loc = (Location *) locations->Get_Next (c);
    while (loc)
    {
      locresult = 0.0;
      if (loc->flags == FLAG_TEXT)
        locresult += text_factor;
      if (loc->flags & FLAG_CAPITAL)
        locresult += caps_factor;
      if (loc->flags & FLAG_TITLE)
        locresult += title_factor;
      if (loc->flags & FLAG_HEADING)
        locresult += heading_factor;
      if (loc->flags & FLAG_KEYWORDS)
        locresult += keywords_factor;
      if (loc->flags & FLAG_DESCRIPTION)
        locresult += meta_desc_factor;
      if (loc->flags & FLAG_AUTHOR)
        locresult += author_factor;
      if (loc->flags & FLAG_LINK_TEXT)
        locresult += description_factor;
      if (loc->flags & FLAG_URL)
        locresult += url_text_factor;

      score += loc->weight * locresult;
      loc = (Location *) locations->Get_Next (c);
    }
  }
  return score;
}
