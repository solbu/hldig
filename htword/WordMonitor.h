//
// WordMonitor.h
//
// NAME
// monitoring classes activity.
//
// SYNOPSIS
//
// Only called thru WordContext::Initialize()
//
// DESCRIPTION
//
// The test directory contains a <i>benchmark-report</i> script used to generate
// and archive graphs from the output of <i>WordMonitor</i>.
//
// CONFIGURATION
//
// wordlist_monitor_period <sec> (default 0)
//   If the value <b>sec</b> is a positive integer, set a timer to
//   print reports every <b>sec</b> seconds. The timer is set using
//   the ALRM signal and will fail if the calling application already
//   has a handler on that signal.
//
// wordlist_monitor_output <file>[,{rrd,readable] (default stderr)
//   Print reports on <b>file</b> instead of the default <b>stderr</b>.
//   If <b>type</b> is set to <b>rrd</b> the output is fit for the
//   <i>benchmark-report</b> script. Otherwise it a (hardly :-) readable
//   string.
//
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordMonitor.h,v 1.5 2004/05/28 13:15:28 lha Exp $
//
#ifndef _WordMonitor_h_
#define _WordMonitor_h_

#include <stdio.h>
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#define WORD_MONITOR_WRITE      1
#define WORD_MONITOR_READ      2
#define WORD_MONITOR_COMPRESS_01    3
#define WORD_MONITOR_COMPRESS_02    4
#define WORD_MONITOR_COMPRESS_03    5
#define WORD_MONITOR_COMPRESS_04    6
#define WORD_MONITOR_COMPRESS_05    7
#define WORD_MONITOR_COMPRESS_06          8
#define WORD_MONITOR_COMPRESS_07          9
#define WORD_MONITOR_COMPRESS_08         10
#define WORD_MONITOR_COMPRESS_09         11
#define WORD_MONITOR_COMPRESS_10         12
#define WORD_MONITOR_COMPRESS_MORE         13
#define WORD_MONITOR_PAGE_IBTREE         14
#define WORD_MONITOR_PAGE_LBTREE         15
#define WORD_MONITOR_PAGE_UNKNOWN         16
#define WORD_MONITOR_PUT           17
#define WORD_MONITOR_GET           18
#define WORD_MONITOR_GET_NEXT           19
#define WORD_MONITOR_GET_SET_RANGE         20
#define WORD_MONITOR_GET_OTHER           21
#define WORD_MONITOR_LEVEL           22
#define WORD_MONITOR_PGNO           23
#define WORD_MONITOR_CMP           24

#define WORD_MONITOR_VALUES_SIZE         50

#ifdef __cplusplus
extern "C"
{
#endif

  void word_monitor_click ();
  void word_monitor_add (int index, unsigned int value);
  void word_monitor_set (int index, unsigned int value);
  unsigned int word_monitor_get (int index);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include "Configuration.h"
#include "htString.h"

class WordMonitor
{
public:
  WordMonitor (const Configuration & config);
   ~WordMonitor ();

  //
  // Unique instance handlers
  //
  static void Initialize (const Configuration & config);
  static WordMonitor *Instance ()
  {
    return instance;
  }

  void Add (int index, unsigned int value)
  {
    values[index] += value;
  }
  void Set (int index, unsigned int value)
  {
    values[index] = value;
  }
  unsigned int Get (int index)
  {
    return values[index];
  }

  const String Report () const;

  void TimerStart ();
  void TimerClick (int signal);
  void TimerStop ();

private:
  unsigned int values[WORD_MONITOR_VALUES_SIZE];
  unsigned int old_values[WORD_MONITOR_VALUES_SIZE];
  time_t started;
  time_t elapsed;
  int period;
  FILE *output;
  int output_style;
  static const char *values_names[WORD_MONITOR_VALUES_SIZE];

  //
  // Unique instance pointer
  //
  static WordMonitor *instance;
};

#endif /* __cplusplus */

#endif /* _WordMonitor_h_ */
