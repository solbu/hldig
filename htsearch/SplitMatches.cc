//
// SplitMatches.cc
//
// SplitMatches:
//  Holds a list of lists with the matches, as specified in
//      search_results_order.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: SplitMatches.cc,v 1.6 2004/05/28 13:15:24 lha Exp $

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "StringList.h"
#include "HtRegex.h"
#include "SplitMatches.h"

#include <stdio.h>
#include <ctype.h>

// This class is only used in private members of SplitMatches.
// The OO-right thing would be to nest this inside the private
// declaration of SplitMatches, but that would cause portability
// problems according to
// <URL:http://www.mozilla.org/hacking/portable-cpp.html#inner_classes>.
//
// It is used as a container for a key (String) and a list.
//
class MatchArea : public Object
{
public:
    // Construct from a string applicable to StringMatch.
    MatchArea(const String &);

    ~MatchArea();

    // Does this item match?
    // Fail if template is empty, since explicit "*" maps to empty template
    inline bool Match(char *s)
    { return match.match(s, 0, 0) != 0; }

    // Return the contained list.
    List *MatchList() { return &myList; }

private:
    HtRegex match;
    List myList;

    // These member functions are not supposed to be implemented, but
    // mentioned here as private so the compiler will not generate them if
    // someone puts in buggy code that would use them.
    MatchArea();
    MatchArea(const MatchArea &);
    void operator= (const MatchArea &);
};

MatchArea::MatchArea(const String &url_regex)
{
    // We do not want to "install" the catch-the-rest pattern as a real
    // pattern; it must always return false for the "Match" operator.
    if (strcmp("*", url_regex.get()) != 0)
      {
  StringList l(url_regex.get(),'|');
  match.setEscaped(l);
      }
}

MatchArea::~MatchArea()
{
}

SplitMatches::SplitMatches(Configuration &config)
{
    const char *config_item = "search_results_order";

    StringList sl(config[config_item], "\t \r\n");

    mySubAreas = new List();
    myDefaultList = 0;

    // Parse each as in TemplateList::createFromString.
    for (int i = 0; i < sl.Count(); i++)
    {
  String sub_area_pattern = sl[i];
  MatchArea *match_item = new MatchArea(sub_area_pattern);
  mySubAreas->Add(match_item);

  // If this is the magic catch-rest sub-area-pattern, we want to
  // use its list-pointer to store all URLs that do not match
  // anything else.
  //  We will iterate over a list where one of the patterns is
  // known to not match, but that's a small penalty for keeping
  // the code simple.
  if (strcmp("*", sub_area_pattern.get()) == 0)
      myDefaultList = match_item->MatchList();
    }

    // If we did not have a catch-the-rest pattern, install one at the
    // end of the list.
    if (myDefaultList == 0)
    {
  MatchArea *match_item = new MatchArea(String("*"));
  mySubAreas->Add(match_item);

  myDefaultList = match_item->MatchList();
    }
}

SplitMatches::~SplitMatches()
{
    // myDefaultList is a pointer to one of the items in mySubAreas and
    // must not be explicitly deleted here.

    delete mySubAreas;
}

void
SplitMatches::Add(ResultMatch *match, char *url)
{
    List *area_list = mySubAreas;
    MatchArea *area_item;

    area_list->Start_Get();

    // This is a linear search.  If there's a problem with that, we
    // can improve it.  For now, a list with tens of areas seems lots,
    // and break-even with a more clever search-scheme is probably in
    // the hundreds.
    while ((area_item = (MatchArea *) area_list->Get_Next()))
    {
  // Use the first match only.
  if (area_item->Match(url))
  {
      area_item->MatchList()->Add(match);
      return;
  }
    }

    // We'll get here if no match was found, so we add to the
    // catch-the-rest list.
    myDefaultList->Add(match);
}

// Just a simple iterator function.
List *
SplitMatches::Get_Next()
{
    MatchArea *next_area = (MatchArea *) mySubAreas->Get_Next();
    List *next_area_list = 0;

    if (next_area != 0)
  next_area_list = next_area->MatchList();

    return next_area_list;
}

// Rip out the sub-areas lists and concatenate them into one list.
List *
SplitMatches::JoinedLists()
{

    // We make a new list here, so we don't have to worry about
    // mySubAreas being dangling or null.
    List *all_areas = new List();
    List *sub_areas = mySubAreas;
    MatchArea *area;

    sub_areas->Start_Get();

    while ((area = (MatchArea *) sub_areas->Get_Next()))
    {
  // "Destructively" move the contents of the list,
  // leaving the original list empty.
  all_areas->AppendList(*(area->MatchList()));
    }

    return all_areas;
}
