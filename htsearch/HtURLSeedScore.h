//
// HtURLSeedScore.h
//
// URLSeedScore:  Constructed from a Configuration, see doc
// for format of config item "url_seed_score".
//  Method "double adjust_score(double score, const String &url)"
// returns an adjusted score, given the original score, or returns the
// original score if there was no adjustment to do.
//
// $Id: HtURLSeedScore.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
#ifndef __HtURLSeedScore_h
#define __HtURLSeedScore_h

#include "Configuration.h"
#include "List.h"

class URLSeedScore
{
public:
    URLSeedScore(Configuration &);
    ~URLSeedScore();

    // Return the "adjusted" score.  Use an inline method to avoid
    // function-call overhead when this feature is unused.
    double adjust_score(double score, const String& url)
    {
	return myAdjustmentList->Count() == 0
	    ? score : noninline_adjust_score(score, url);
    }

    // If an error was discovered during the parsing of
    // the configuration, this member gives a
    // nonempty String with an error message.
    const String& ErrMsg() { return myErrMsg; }

private:
    double noninline_adjust_score(double score, const String& url);

    // These member functions are not supposed to be implemented.
    URLSeedScore();
    URLSeedScore(const URLSeedScore &);
    void operator= (const URLSeedScore &);

    List *myAdjustmentList;
    String myErrMsg;
};

#endif /* __HtURLSeedScore_h */
