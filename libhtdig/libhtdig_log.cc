//--------------------------------------------------------------------
//
// libhtdig_log.cc
//
// 2/6/2002 created
//
// Neal Richter nealr@rightnow.com
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_log.cc,v 1.3 2003/06/24 19:56:52 nealr Exp $
//
//--------------------------------------------------------------------

#include "libhtdig_log.h"


#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <fstream.h>
#include <stdio.h>
#include <time.h>


static FILE *errorlog_fp = NULL;

int  logOpen(char *filename)
{
    if(errorlog_fp == NULL)
        errorlog_fp = fopen(filename, "a+");

    if (errorlog_fp == NULL)
    	return (TRUE);
    else
	    return (FALSE);
}


void logEntry (char *msg)
{
    time_t now = time(NULL);
    
    if(errorlog_fp != NULL)
    	fprintf(errorlog_fp, "[%s] %s\n", ctime(&now), msg);

}


//*****************************************************************************
// Report an error

void reportError (char *msg)
{
    time_t now = time(NULL);
    
    if(errorlog_fp != NULL)
    	fprintf(errorlog_fp, "%s  [ERROR] %s\n", ctime(&now), msg);

    fprintf(stderr, "%s  [ERROR] %s\n", ctime(&now), msg);
    
}


int logClose()
{
    int ret = -1;

    if(errorlog_fp != NULL)
    {
        ret = fclose(errorlog_fp);
        errorlog_fp = NULL;

        if(ret == 0)
            return(TRUE);
        else
            return(FALSE);
    }

    return(TRUE);
}
