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
// $Id: HtMaxMin.h,v 1.1.2.3 2000/05/10 18:23:44 loic Exp $
//

#ifndef _HtMaxMin_h_
#define _HtMaxMin_h_

#define HtMAX(a,b) (((a)>(b)) ? (a) : (b))
#define HtMIN(a,b) (((a)<(b)) ? (a) : (b))


// Max/Min value of an array
class HtMaxMin
{
 public:
    // compute max/min of an array of values
    static unsigned int   max_v(unsigned int   *vals,int n);
    static unsigned short max_v(unsigned short *vals,int n);
    static unsigned int   min_v(unsigned int   *vals,int n);
    static unsigned short min_v(unsigned short *vals,int n);
};



#endif
