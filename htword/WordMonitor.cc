//
// WordMonitor.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordMonitor.cc,v 1.1.2.2 2000/01/03 11:48:36 bosc Exp $
//
#include<stdlib.h>

#include"WordMonitor.h"
#include"WordList.h"
#include"WordDBCompress.h"
#include"HtMaxMin.h"
WordMonitorOutput::WordMonitorOutput(const Configuration &config)
{
    all_fields = 0;

    // parse period options
    const String &periodstr = config["wordlist_monitor_period"];

    if( periodstr.empty() ){ period = 1.0; }
    else { period = atof((const char *)periodstr); }
    

    // parse fields
    String fieldsstr = config["wordlist_monitor_fields"];

    if( fieldsstr.empty() ){ fieldsstr="all"; }

    StringList sfields(fieldsstr, "\t ");

    sfields.Start_Get();

    String *found;
    while((found = (String*)sfields.Get_Next()))
    {
	if( *found == (String)"all" )
	{
	    all_fields = 1;
	    break;
	}
	fields.push_back(*found);
    }

    // parse output fields
    String outputstr = config["wordlist_monitor_output"];
    out=NULL;
    if(!strncmp((char *)outputstr,"file:",sizeof("file:")))
    {
	out=fopen(((char *)outputstr)+sizeof("file:"),"w");
	if(!out){cerr << "WordMonitorOutput::WordMonitorOutput: invalid output file" << endl;}
    }

    if(!out){out=stdout;}

}


WordMonitorOutput::~WordMonitorOutput()
{
    if( out != stdout ){fclose(out);}
}


void
WordMonitorOutput::put(const String &name,const String &value)
{
    pending_out.Add(name,new String(value));
}

void
WordMonitorOutput::put(const String &name,double val)
{
    char s[30];
    sprintf(s,"%f",val);
    String str(s);
    put(name, str);
}
void
WordMonitorOutput::put(const String &name,int val)
{
    char s[30];
    sprintf(s,"%6d",val);
    String str(s);
    put(name, str);
}

void
WordMonitorOutput::go()
{
    int i=0;
    pending_out.Start_Get();
    while(1)
    {
	char *name;
	if(all_fields){name=pending_out.Get_Next();}
	else{name=(i<fields.size() ? (char *)fields[i] : (char *)NULL);}
	if(!name){break;}
//  	cerr << "WordMonitorOutput::go: i:" << i << "name:" << name << endl;
	String *valp=(String *)pending_out[name];
	String val;
	if(!valp){val="???";}
	else{val = *valp;}
	fprintf(out,"%s:%s ",name,(char *)val);
	i++;
    }
    fprintf(out,"\n");


    pending_out.Start_Get();
    String *pstr=NULL;
    while((pstr=(String *)pending_out.Get_NextElement()))
    {
	delete pstr;
    }

    pending_out.Release();
}


WordMonitor::WordMonitor(const Configuration &config,WordDBCompress *ncmpr,WordList *nwlist):
    output(config),
    periodic(output.period)
{
    cmpr=ncmpr;
    wlist=nwlist;
    nomonitor=0;
    if(config["wordlist_monitor"].empty()){nomonitor=1;}

    dbc_last_cmpr_count = 0;
    dbc_last_cmpr_time  = 0;
    dbc_last_ucmpr_count= 0;
    dbc_last_ucmpr_time = 0;
    dbc_last_mxtreelevel= 0;
    dbc_last_nonleave_count=0;
    dbc_last_cmpr_ratio=0;
    dbc_last_cmpr_overflow=0;
    
    wl_last_put_count              = 0;
    wl_last_put_time               = 0;
    wl_last_walk_count             = 0;
    wl_last_walk_time              = 0;
    wl_last_walk_count_DB_SET_RANGE= 0;
    wl_last_walk_count_DB_NEXT     = 0;
}

void 
WordMonitor::process(double rperiod)
{

    int    wl_diff_put_count              =wlist->bm_put_count              - wl_last_put_count              ;
    double wl_diff_put_time               =wlist->bm_put_time               - wl_last_put_time               ;
    int    wl_diff_walk_count             =wlist->bm_walk_count             - wl_last_walk_count             ;
    double wl_diff_walk_time              =wlist->bm_walk_time              - wl_last_walk_time              ;
    int    wl_diff_walk_count_DB_SET_RANGE=wlist->bm_walk_count_DB_SET_RANGE- wl_last_walk_count_DB_SET_RANGE;
    int    wl_diff_walk_count_DB_NEXT     =wlist->bm_walk_count_DB_NEXT     - wl_last_walk_count_DB_NEXT     ;


    if(cmpr)
    {
	int    dbc_diff_cmpr_count     = cmpr->bm_cmpr_count     - dbc_last_cmpr_count    ;
	double dbc_diff_cmpr_time      = cmpr->bm_cmpr_time      - dbc_last_cmpr_time     ;
	int    dbc_diff_ucmpr_count    = cmpr->bm_ucmpr_count    - dbc_last_ucmpr_count   ;
	double dbc_diff_ucmpr_time     = cmpr->bm_ucmpr_time     - dbc_last_ucmpr_time    ;
//  	int    dbc_diff_mxtreelevel    = cmpr->bm_mxtreelevel    - dbc_last_mxtreelevel   ;
	int    dbc_diff_nonleave_count = cmpr->bm_nonleave_count - dbc_last_nonleave_count;
	double dbc_diff_cmpr_ratio     = cmpr->bm_cmpr_ratio      - dbc_last_cmpr_ratio         ;
	int    dbc_diff_cmpr_overflow  = cmpr->bm_cmpr_overflow   - dbc_last_cmpr_overflow      ;

	
	output.put( "mxtreelevel"         , cmpr->bm_mxtreelevel );
	output.put( "cmpr/s"              , dbc_diff_cmpr_count             / rperiod );
	output.put( "ucmpr/s"             , dbc_diff_ucmpr_count            / rperiod );
	output.put( "cmpr_ucmpr_time"     , (dbc_diff_cmpr_time + dbc_diff_ucmpr_time) / rperiod );
	output.put( "nonleave/leave"     , (dbc_diff_nonleave_count) / (double)HtMAX(1,(dbc_diff_ucmpr_count+dbc_diff_cmpr_count )));
	output.put( "cmpr_ratio"     , dbc_diff_cmpr_ratio / (double)HtMAX(1,dbc_diff_cmpr_count ));
	output.put( "cmpr_overflow"       , dbc_diff_cmpr_overflow /(double)HtMAX(1,dbc_diff_cmpr_count ));
    }
    

    output.put( "totput"              , wlist->bm_put_count );
    output.put( "put/s"               , wl_diff_put_count  / rperiod );
    output.put( "put_time"            , wl_diff_put_time   / rperiod );
    output.put( "nwalks/s"            , wl_diff_walk_count / rperiod );
    output.put( "walk_time"           , wl_diff_walk_time  / rperiod );
    output.put( "nwalk_set"           , wlist->bm_walk_count_DB_SET_RANGE );
    output.put( "nwalk_set/s"         , wl_diff_walk_count_DB_SET_RANGE / rperiod );
    output.put( "nwalk_next"          , wlist->bm_walk_count_DB_NEXT );
    output.put( "nwalk_next/s"        , wl_diff_walk_count_DB_NEXT      / rperiod );

    output.go();

    if(cmpr)
    {
	dbc_last_cmpr_count     = cmpr->bm_cmpr_count   ;
	dbc_last_cmpr_time      = cmpr->bm_cmpr_time    ;
	dbc_last_ucmpr_count    = cmpr->bm_ucmpr_count  ;
	dbc_last_ucmpr_time     = cmpr->bm_ucmpr_time   ;
	dbc_last_mxtreelevel    = cmpr->bm_mxtreelevel  ;
	dbc_last_cmpr_ratio     = cmpr->bm_cmpr_ratio   ;
	dbc_last_cmpr_overflow  = cmpr->bm_cmpr_overflow;
    }

    wl_last_put_count              = wlist->bm_put_count              ;
    wl_last_put_time               = wlist->bm_put_time               ;
    wl_last_walk_count             = wlist->bm_walk_count             ;
    wl_last_walk_time              = wlist->bm_walk_time              ;
    wl_last_walk_count_DB_SET_RANGE= wlist->bm_walk_count_DB_SET_RANGE;
    wl_last_walk_count_DB_NEXT     = wlist->bm_walk_count_DB_NEXT     ;
}





