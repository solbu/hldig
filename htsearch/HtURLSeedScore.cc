//
// HtURLSeedScore.cc
//
// URLSeedScore:
//	Holds a list of configured adjustments to be applied on a given
//	score and given URL.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtURLSeedScore.cc,v 1.3 2002/06/14 22:31:39 grdetil Exp $

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "StringList.h"
#include "HtRegex.h"
#include "HtURLSeedScore.h"

#include <stdio.h>
#include <ctype.h>

// This class is only used in private members of URLSeedScore.
// The OO-right thing would be to nest this inside the private
// declaration of HtURLSeedScore, but that would cause portability
// problems according to
// <URL:http://www.mozilla.org/hacking/portable-cpp.html#inner_classes>.

class ScoreAdjustItem : public Object
{
public:
    // Construct from a string applicable to StringMatch, and a string to
    // parse for a formula.
    ScoreAdjustItem(String &, String &);

    ~ScoreAdjustItem();

    // Does this item match?
    inline bool Match(const String &s) { return match.match(s, 1, 0) != 0; }

    // Return the argument adjusted according to this item.
    double adjust_score(double orig)
    { return orig*my_mul_factor + my_add_constant; }

    // Error in parsing?  Message given here if non-empty string.
    String& ErrMsg() { return myErrMsg; }

private:
    double my_add_constant;
    double my_mul_factor;
    HtRegex match;

    static String myErrMsg;

    // These member functions are not supposed to be implemented, but
    // mentioned here as private so the compiler will not generate them if
    // someone puts in buggy code that would use them.
    ScoreAdjustItem();
    ScoreAdjustItem(const ScoreAdjustItem &);
    void operator= (const ScoreAdjustItem &);
};

// Definition of myErrMsg.
String ScoreAdjustItem::myErrMsg("");

ScoreAdjustItem::ScoreAdjustItem(String &url_regex, String &formula)
{
    double mul_factor = 1;
    double add_constant = 0;
    bool factor_found = false;
    bool constant_found = false;
    int chars_so_far;
    StringList l(url_regex.get());
    match.setEscaped(l);

    // FIXME: Missing method to check if the regex was in error.
    //	myErrMsg = form("%s is not a valid regex", url_regex.get());

    char *s = formula.get();

    // Parse the ([*]N[ ]*)?[+]?M format.
    if (s[0] == '*')
    {
	// Skip past the '*'.
	s++;

	// There is a mul_factor.  Let's parse it.
	chars_so_far = 0;
	sscanf(s, "%lf%n", &mul_factor, &chars_so_far);

	// If '%lf' failed to match, then it will show up as either no
	// assignment to chars_so_far, or as writing 0 there.
	if (chars_so_far == 0)
	{
	    myErrMsg = form("%s is not a valid adjustment formula", s);
	    return;
	}

	// Skip past the number.
	s += chars_so_far;

	// Skip any whitespaces.
	while (isspace(*s))
	    s++;

	// Eat any plus-sign; it's redundant if alone, and may come before a
	// minus.
	if (*s == '+')
	    s++;

	factor_found = true;
    }

    // If there's anything here, it must be the additive constant.
    if (*s)
    {
	chars_so_far = 0;
	sscanf(s, "%lf%n", &add_constant, &chars_so_far);

	// If '%lf' failed to match, then it will show up as either no
	// assignment to chars_so_far, or as writing 0 there.
	//  We also need to check that it was the end of the input.
	if (chars_so_far == 0 || s[chars_so_far] != 0)
	{
	    myErrMsg = form("%s is not a valid adjustment formula",
			    formula.get());
	    return;
	}

	constant_found = true;
    }

    // Either part must be there.
    if (!factor_found && !constant_found)
    {
	myErrMsg = form("%s is not a valid formula", formula.get());
	return;
    }

    my_add_constant = add_constant;
    my_mul_factor = mul_factor;
}

ScoreAdjustItem::~ScoreAdjustItem()
{
}

URLSeedScore::URLSeedScore(Configuration &config)
{
    char *config_item = "url_seed_score";

    StringList sl(config[config_item], "\t \r\n");

    myAdjustmentList = new List();

    if (sl.Count() % 2)
    {
	myErrMsg = form("%s is not a list of pairs (odd number of items)",
			config_item);

	// We *could* continue, but that just means the error will be harder
	// to find, unless someone actually sees the error message.
	return;
    }

    // Parse each as in TemplateList::createFromString.
    for (int i = 0; i < sl.Count(); i += 2)
    {
	String url_regex = sl[i];
	String adjust_formula = sl[i+1];

	ScoreAdjustItem *adjust_item
	    = new ScoreAdjustItem(url_regex, adjust_formula);

	if (adjust_item->ErrMsg().length() != 0)
	{
	    // No point in continuing beyond the error; we might just
	    // overwrite the first error.
	    myErrMsg = form("While parsing %s: %s",
			    config_item,
			    adjust_item->ErrMsg().get());
	    return;
	}

	myAdjustmentList->Add(adjust_item);
    }
}

URLSeedScore::~URLSeedScore()
{
    delete myAdjustmentList;
}

double
URLSeedScore::noninline_adjust_score(double orig_score, const String &url)
{
    List *adjlist = myAdjustmentList;
    ScoreAdjustItem *adjust_item;

    adjlist->Start_Get();

    while ((adjust_item = (ScoreAdjustItem *) adjlist->Get_Next()))
    {
	// Use the first match only.
	if (adjust_item->Match(url))
	    return adjust_item->adjust_score(orig_score);
    }

    // We'll get here if no match was found.
    return orig_score;
}
