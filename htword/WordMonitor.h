//
// WordMonitor.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordMonitor.h,v 1.1.2.3 2000/01/11 18:37:35 bosc Exp $
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
    void SetOutputFields(const String &sfields);

    void put(const String &name,const String &value);// add output value
    void put(const String &name,double val);// add output value
    void put(const String &name,int val);// add output value
    void go();// do output

    double period; // sampling period
    WordMonitorOutput(const Configuration &config);
    ~WordMonitorOutput();
};

class CommandProcessor
{
public:
    virtual int ProcessCommand(const String& command)=0;
};


// command line input 
class WordMonitorInput
{
    CommandProcessor *commandProcessor;
    HtTime::Periodic periodic;    
    FILE *fin;
    const char *ifname;
    int inputpos;
    void ParseInput();
 public:
    inline void operator () ()
    {
	if(periodic()){ParseInput();}
    }
    WordMonitorInput(const Configuration &config, CommandProcessor *ncommandProcessor);
};
class WordList;
class WordDBCompress;

// generates  mifluz-specific  monitor data 
class WordMonitor : public CommandProcessor
{
    int nomonitor;
    WordMonitorInput  *input;
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
    virtual int ProcessCommand(const String& command);
    inline void operator () ()
    {
	if(nomonitor){return;}
	double rperiod;
	if(periodic(&rperiod)){process(rperiod);}
	if(input){(*input)();}
    }
    virtual ~WordMonitor(){;}
    WordMonitor(const Configuration &config,WordDBCompress *ncmpr,WordList *nwlist);
};



#endif
