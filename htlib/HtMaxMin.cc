// HtMaxMin
//
// macros and tools for computing max and min of values
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtMaxMin.cc,v 1.2 2000/02/19 05:29:03 ghutchis Exp $
//

#include"HtMaxMin.h"

unsigned int
HtMaxMin::max_v(unsigned int *vals,int n)
{
    unsigned int maxv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned int v=vals[i];
	if(v>maxv){maxv=v;}
    }
    return(maxv);
}

unsigned short
HtMaxMin::max_v(unsigned short *vals,int n)
{
    unsigned short maxv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned short v=vals[i];
	if(v>maxv){maxv=v;}
    }
    return(maxv);
}

unsigned int
HtMaxMin::min_v(unsigned int *vals,int n)
{
    unsigned int minv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned int v=vals[i];
	if(v<minv){minv=v;}
    }
    return(minv);
}

unsigned short
HtMaxMin::min_v(unsigned short *vals,int n)
{
    unsigned short minv=vals[0];
    for(int i=1;i<n;i++)
    {
	unsigned short v=vals[i];
	if(v<minv){minv=v;}
    }
    return(minv);
}
