//
// WordMonitor.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordMonitor.cc,v 1.1.2.11 2000/09/21 04:25:36 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "StringList.h"
#include "WordMonitor.h"

#define WORD_MONITOR_RRD	1
#define WORD_MONITOR_READABLE	2

char* WordMonitor::values_names[WORD_MONITOR_VALUES_SIZE] = {
  "",
  "C.Write",
  "C.Read",
  "C.Compress 1/1",
  "C.Compress 1/2",
  "C.Compress 1/3",
  "C.Compress 1/4",
  "C.Compress 1/5",
  "C.Compress 1/6",
  "C.Compress 1/7",
  "C.Compress 1/8",
  "C.Compress 1/9",
  "C.Compress 1/10",
  "C.Compress 1/>10",
  "C.P_IBTREE",
  "C.P_LBTREE",
  "C.P_UNKNOWN",
  "C.Put",
  "C.Get (0)",
  "C.Get (NEXT)",
  "C.Get (SET_RANGE)",
  "C.Get (Other)",
  "G.LEVEL",
  "G.PGNO",
  "C.CMP",
  0
};

WordMonitor::WordMonitor(const Configuration &config)
{
  memset((char*)values, '\0', sizeof(unsigned int) * WORD_MONITOR_VALUES_SIZE);
  memset((char*)old_values, '\0', sizeof(unsigned int) * WORD_MONITOR_VALUES_SIZE);
  started = elapsed = time(0);
  output_style = WORD_MONITOR_READABLE;
  if((period = config.Value("wordlist_monitor_period"))) {
    const String& desc = config.Find("wordlist_monitor_output");
    StringList fields(desc, ',');

    if(fields.Count() > 0) {
      char* filename = fields[0];
      if(filename[0] == '\0')
	output = stderr;
      else {
	output = fopen(filename, "a");
	if(!output) {
	  fprintf(stderr, "WordMonitor::WordMonitor: cannot open %s for writing ", filename);
	  perror("");
	  output = stderr;
	  return;
	}
      }
      if(fields.Count() > 1) {
	char* style = fields[1];
	if(!strcasecmp(style, "rrd"))
	  output_style = WORD_MONITOR_RRD;
	else
	  output_style = WORD_MONITOR_READABLE;
      }
    }
    Start();
  }
}

WordMonitor::~WordMonitor()
{
  Stop();
  if(output != stderr)
    fclose(output);
}

const String
WordMonitor::Report() const
{
  String output;
  int i;
  time_t now = time(0);

  if(output_style == WORD_MONITOR_RRD)
    output << (int)now << ":";

  for(i = 0; i < WORD_MONITOR_VALUES_SIZE; i++) {
    if(!values_names[i]) break;
    if(values_names[i][0]) {
      if(output_style == WORD_MONITOR_READABLE) {
	output << values_names[i] << ": " << values[i];
	if((now - elapsed) > 0) {
	  output << ", per sec : " << (int)(values[i] / (now - started));
	  output << ", delta : " << (values[i] - old_values[i]);
	  output << ", per sec : " << (int)((values[i] - old_values[i]) / (now - elapsed));
	}
	output << "|";
      } else if(output_style == WORD_MONITOR_RRD) {
	output << values[i] << ":";
      }
    }
  }
  memcpy((char*)old_values, (char*)values, sizeof(unsigned int) * WORD_MONITOR_VALUES_SIZE);
  return output;
}

void 
WordMonitor::Start()
{
  if(period < 5) {
    fprintf(stderr, "WordMonitor::Start: wordlist_monitor_period must be > 5 (currently %d) otherwise monitoring is not accurate\n", period);
    return;
  }

  fprintf(output, "----------------- WordMonitor starting -------------------\n");
  if(output_style == WORD_MONITOR_RRD) {
    fprintf(output, "Started:%ld\n", started);
    fprintf(output, "Period:%d\n", period);
    fprintf(output, "Time:");
    int i;
    for(i = 0; i < WORD_MONITOR_VALUES_SIZE; i++) {
      if(!values_names[i]) break;
      if(values_names[i][0])
	fprintf(output, "%s:", values_names[i]);
    }
    fprintf(output, "\n");
  }
  fflush(output);
}

void
WordMonitor::Click()
{
  //
  // Do not report if less than <period> since last report.
  //
  if(time(0) - elapsed >= period) {
    fprintf(output, "%s\n", (const char*)Report());
    elapsed = time(0);
    fflush(output);
  }
}

void
WordMonitor::Stop()
{
  if(period > 0) {
    //
    // Make sure last report is at least one second older than the previous one.
    //
    if(time(0) - elapsed < 1)
      sleep(2);
    fprintf(output, "%s\n", (const char*)Report());
    fprintf(output, "----------------- WordMonitor finished -------------------\n");
  }
}

//
// C interface to WordMonitor instance
//

extern "C" {
  void word_monitor_click(void *nmonitor)
  {
    WordMonitor* monitor = (WordMonitor*)nmonitor;
    if(monitor)
      monitor->Click();
  }
  void word_monitor_add(void *nmonitor, int index, unsigned int value) 
  {
    WordMonitor* monitor = (WordMonitor*)nmonitor;
    if(monitor)
      monitor->Add(index, value);
  }
  void word_monitor_set(void *nmonitor, int index, unsigned int value)
  {
    WordMonitor* monitor = (WordMonitor*)nmonitor;
    if(monitor)
      monitor->Set(index, value);
  }
  unsigned int word_monitor_get(void *nmonitor, int index)
  {
    WordMonitor* monitor = (WordMonitor*)nmonitor;
    if(monitor)
      return monitor->Get(index);
    else
      return 0;
  }
}
