//
// HtRegexReplace.cc
//
// HtRegexReplace: A subclass of HtRegex that can perform replacements
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegexReplace.cc,v 1.1.4.1 2001/09/27 22:02:11 grdetil Exp $
//

#include "HtRegexReplace.h"
#include <locale.h>


HtRegexReplace::HtRegexReplace()
{
}

HtRegexReplace::HtRegexReplace(const char *from, const char *to, int case_sensitive)
	: HtRegex(from, case_sensitive)
{
	memset(&regs, 0, sizeof(regs));
	repBuf		= 0;
	segSize		=
	segUsed		= 0;
	segMark		= 0;
	repLen		= 0;

	setReplace(to);
}

HtRegexReplace::~HtRegexReplace()
{
	empty();
}

int HtRegexReplace::replace(String &str, int nullpattern, int nullstr)
{
	const int regCount = sizeof(regs) / sizeof(regs[0]);
	if (compiled == 0 || repBuf == 0) return nullpattern;
	if (str.length() == 0) return nullstr;

	if (regexec(&re, str.get(), regCount, regs, 0) == 0)
	{
		// Firstly work out how long the result string will be. We think this will be more effecient
		// than letting the buffer grow in stages as we build the result, but who knows?
		//cout << "!!! Match !!!" << endl;
		size_t resLen = repLen;
		int i, reg, repPos;
		const char *src = str.get();

		for (i = 1; i < (int) segUsed; i += 2)
		{
			reg = segMark[i];
			if (reg < regCount && regs[reg].rm_so != -1)
				resLen += regs[reg].rm_eo - regs[reg].rm_so;
		}
		//cout << "result will be " << resLen << " chars long" << endl;
		String result(resLen);	// Make the result string preallocating the buffer size
		for (i = 0, repPos = 0;; )
		{
			//cout << "appending segment " << i << endl;
			result.append(repBuf + repPos, segMark[i] - repPos);		// part of the replace string
			repPos = segMark[i];		// move forward
			if (++i == (int) segUsed) break;	// was that the last segment?
			reg = segMark[i++];			// get the register number
			if (reg < regCount && regs[reg].rm_so != -1)
				result.append((char *) src + regs[reg].rm_so, regs[reg].rm_eo - regs[reg].rm_so);
		}
		str = result;
		//cout << "return " << result.get() << endl;

		return 1;
	}

	return 0;
}

// Private: place a mark in the mark buffer growing it if necessary.
void HtRegexReplace::putMark(int n)
{
	// assert(segUsed <= segSize);
	if (segUsed == segSize)
	{
		size_t newSize = segSize * 2 + 5;		// grow in chunks
		int *newMark = new int[newSize];		// do we assume that new can't fail?
		memcpy(newMark, segMark, segSize * sizeof(int));
		delete segMark;
		segMark = newMark;
		segSize = newSize;
	}
	segMark[segUsed++] = n;
}

void HtRegexReplace::empty()
{
	// Destroy any existing replace pattern
    delete repBuf; repBuf = 0;
    segSize = segUsed = 0;
    delete segMark; segMark = 0;
    repLen = 0;
}

void HtRegexReplace::setReplace(const char *to)
{
	empty();

	repBuf = new char[strlen(to)];		// replace buffer can never contain more text than to string
	int bufPos = 0;			// our position within the output buffer

	while (*to)
	{
		if (*to == '\\')
		{
			if (*++to == '\0') break;
			if (*to >= '0' && *to <= '9')
			{
				putMark(bufPos);
				putMark(*to - '0');
			}
			else
			{
				// We could handle some C style escapes here, but instead we just pass the character
				// after the backslash through. This means that \\, \" and \' will do the right thing.
				// It's unlikely that anyone will need any C style escapes in ht://Dig anyway.
				repBuf[bufPos++] = *to;
			}
			to++;
		}
		else
		{
			repBuf[bufPos++] = *to++;
		}
	}
	putMark(bufPos);
	repLen = (size_t) bufPos;
}
