//
// WordMonitor.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordMonitor.h,v 1.1.2.1 2000/01/03 10:02:06 bosc Exp $
//
#ifndef _WordMonitor_h_
#define _WordMonitor_h_

#include"HtTime.h"
#include"Configuration.h"
#include"HtVector_String.h"

// filter that alows user selctable/configurable output monitoring
class WordMonitorOutput
{
    
    Dictionary pending_out;
    int all_fields; // show all fields
    HtVector_String fields;
    FILE *out;

 public:

    void put(const String &name,const String &value);// add output value
    void put(const String &name,double val);// add output value
    void put(const String &name,int val);// add output value
    void go();// do output

    double period; // sampling period
    WordMonitorOutput(const Configuration &config);
    ~WordMonitorOutput();
};

class WordList;
class WordDBCompress;

// generates  mifluz-specific  monitor data 
class WordMonitor
{
    int nomonitor;
    WordMonitorOutput output;
    HtTime::Periodic periodic;
    WordDBCompress *cmpr;
    WordList *wlist;
    void process(double rperiod);

// WordDBCompress info
    int    dbc_last_cmpr_count ;
    double dbc_last_cmpr_time  ;
    int    dbc_last_ucmpr_count;
    double dbc_last_ucmpr_time ;
    int    dbc_last_mxtreelevel;
    int    dbc_last_nonleave_count;
    double dbc_last_cmpr_ratio;
    int    dbc_last_cmpr_overflow;

// WordList info
    int    wl_last_put_count              ;
    double wl_last_put_time               ;
    int    wl_last_walk_count             ;
    double wl_last_walk_time              ;
    int    wl_last_walk_count_DB_SET_RANGE;
    int    wl_last_walk_count_DB_NEXT     ;

 public:    
    inline void operator () ()
    {
	if(nomonitor){return;}
	double rperiod;
	if(periodic(&rperiod)){process(rperiod);}
    }
    WordMonitor(const Configuration &config,WordDBCompress *ncmpr,WordList *nwlist);
};



#endif
