// HtMaxMin
//
// macros and tools for computing max and min of values
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtMaxMin.cc,v 1.1.2.3 2000/09/14 03:13:24 ghutchis Exp $
//

#include"HtMaxMin.h"

unsigned int HtMaxMin::max_v(unsigned int *vals, int n)
{
    unsigned int maxv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned int v = vals[i];
	if (v > maxv) {
	    maxv = v;
	}
    }
    return (maxv);
}

unsigned short HtMaxMin::max_v(unsigned short *vals, int n)
{
    unsigned short maxv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned short v = vals[i];
	if (v > maxv) {
	    maxv = v;
	}
    }
    return (maxv);
}

unsigned char HtMaxMin::max_v(unsigned char *vals, int n)
{
    unsigned char maxv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned char v = vals[i];
	if (v > maxv) {
	    maxv = v;
	}
    }
    return (maxv);
}

unsigned int HtMaxMin::min_v(unsigned int *vals, int n)
{
    unsigned int minv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned int v = vals[i];
	if (v < minv) {
	    minv = v;
	}
    }
    return (minv);
}

unsigned short HtMaxMin::min_v(unsigned short *vals, int n)
{
    unsigned short minv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned short v = vals[i];
	if (v < minv) {
	    minv = v;
	}
    }
    return (minv);
}

unsigned char HtMaxMin::min_v(unsigned char *vals, int n)
{
    unsigned char minv = vals[0];
    for (int i = 1; i < n; i++) {
	unsigned char v = vals[i];
	if (v < minv) {
	    minv = v;
	}
    }
    return (minv);
}
