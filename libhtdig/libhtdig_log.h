//--------------------------------------------------------------------
//
// libhtdig_log.h
//
// 2/6/2002 created
//
// Neal Richter nealr@rightnow.com
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_log.h,v 1.2 2003/06/23 22:28:17 nealr Exp $
//
//--------------------------------------------------------------------

#ifndef LIBHTDIG_LOG_H
#define LIBHTDIG_LOG_H


#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


int  logOpen(char *file);
void logEntry(char *msg);
void reportError(char *msg);
int logClose(void);

#endif /* LIBHTDIG_LOG_H */

