//
// HtRegexReplace.h
//
// HtRegexReplace: A subclass of HtRegex that can perform replacements
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegexReplace.h,v 1.1.4.1 2001/09/27 22:02:11 grdetil Exp $
//

#ifndef	_HtRegexReplace_h_
#define	_HtRegexReplace_h_

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegex.h"

class HtRegexReplace : public HtRegex
{
public:
    //
    // Construction/Destruction
    //
    HtRegexReplace();
    HtRegexReplace(const char *from, const char *to, int case_sensitive = 0);
    virtual ~HtRegexReplace();

    //
    // Methods for setting the replacement pattern
    //
	void	setReplace(const String& str) { setReplace(str.get()); }
	void	setReplace(const char *str);

    //
    // Methods for replacing
    //
    int		replace(String &str, int nullpattern = 0, int nullstr = 0);

protected:
	char	*repBuf;      // Replace text.
	size_t	segSize, segUsed;
	int		*segMark;
	size_t	repLen;

    regmatch_t		regs[10];

	// Various private methods
	void putMark(int n);
	void empty();
};

#endif
