// HtTime.h
//
// class HtTime:
// tools for timing 
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtTime.h,v 1.1.2.4 2000/05/10 18:23:45 loic Exp $
//
#ifndef _HtTime_h_
#define _HtTime_h_

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


class HtTime
{
 public:
    // time in seconds (double format)
    static inline double DTime()
    {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return(tv.tv_usec/1000000.0+tv.tv_sec);
    }
    // time in seconds relative to T0 (double format)
    static inline double DTime(double T0)
    {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return((tv.tv_usec/1000000.0+tv.tv_sec)-T0);
    }

    // Do something every x seconds
    class Periodic
    {
	double t0;
	double last;
	double period;
    public:
	double total(){return(HtTime::DTime(t0));}
	void change_period(double nperiod){period=nperiod;}
	int operator()(double *prperiod=NULL)
	{
	    double t=HtTime::DTime(t0);
	    if(prperiod){*prperiod=t-last;}
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
    // print progression message every x seconds
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




