// HtRandom.h
//
// class HtRandom:
// tools for random numbers 
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRandom.h,v 1.1.2.2 2000/05/10 18:23:44 loic Exp $
//

#ifndef _HtRandom_h_
#define _HtRandom_h_


class HtRandom
{
 public:
    // produce a random unsigned int between v0 and v1
    static inline unsigned int rnd(unsigned int v0,unsigned int v1)
    {
	return((rand()%(v1-v0)) + v0 );
    }

    // randomly mix up an array of int's
    static int *randomize_v(int *vals,int n)
    {
	int i;
	if(!vals)
	{
	    vals=new int[n];
	    for(i=0;i<n;i++){vals[i]=i;}
	}
	for(i=0;i<2*n;i++)
	{
	    int i0=HtRandom::rnd(0,n);
	    int i1=HtRandom::rnd(0,n);
	    int t=vals[i0];
	    vals[i0]=vals[i1];
	    vals[i1]=t;
	}
	return(vals);
    }

};
#endif // _HtRandom_h_
