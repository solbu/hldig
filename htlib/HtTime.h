// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtTime.h,v 1.1.2.2 1999/12/21 19:55:14 bosc Exp $
//
#ifndef _HtTime_h_
#define _HtTime_h_

#include <time.h>
#include <sys/time.h>

class HtTime
{
 public:
    static inline double DTime()
    {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return(tv.tv_usec/1000000.0+tv.tv_sec);
    }
    static inline double DTime(double T0)
    {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return((tv.tv_usec/1000000.0+tv.tv_sec)-T0);
    }


    class Periodic
    {
	double t0;
	double last;
	double period;
    public:
	double total(){return(HtTime::DTime()-t0);}
	void change_period(double nperiod){period=nperiod;}
	int operator()()
	{
	    double t=HtTime::DTime()-t0;
	    if((t-last)>period)
	    {
		last=t;
		return(1);
	    }
	    return(0);
	}
	Periodic(double nperiod=.1)
	{
	    period=nperiod;
	    t0=HtTime::DTime();
	    last=0;
	}
    };


#ifdef NOTDEF
    class Progression
    {
	double t0;
	double last;
	double period;
	char *label;
    public:
	double total(){return(HtTime::DTime()-t0);}
	int operator()(double x)
	{
	    double t=HtTime::DTime()-t0;
	    if((t-last)>period)
	    {
		last=t;
		printf("%s (%f): %f\n",label,t,x);
		return(1);
	    }
	    return(0);
	}
	Progression(double nperiod=.1,char *nlabel="progression")
	{
	    label=nlabel;
	    period=nperiod;
	    t0=HtTime::DTime();
	    last=0;
	}
    };
#endif
};
#endif // _HtTime_h_




