//
// WordMonitor.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordMonitor.cc,v 1.7 2004/05/28 13:15:28 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <signal.h>

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif

#include "StringList.h"
#include "WordMonitor.h"

#define WORD_MONITOR_RRD  1
#define WORD_MONITOR_READABLE  2

WordMonitor *
  WordMonitor::instance = 0;

const char *
  WordMonitor::values_names[WORD_MONITOR_VALUES_SIZE] = {
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

WordMonitor::WordMonitor (const Configuration & config)
{
  memset ((char *) values, '\0',
          sizeof (unsigned int) * WORD_MONITOR_VALUES_SIZE);
  memset ((char *) old_values, '\0',
          sizeof (unsigned int) * WORD_MONITOR_VALUES_SIZE);
  started = elapsed = time (0);
  output_style = WORD_MONITOR_READABLE;
  if ((period = config.Value ("wordlist_monitor_period")))
  {
    const String & desc = config.Find ("wordlist_monitor_output");
    StringList fields (desc, ',');

    if (fields.Count () > 0)
    {
      char *filename = fields[0];
      if (filename[0] == '\0')
        output = stderr;
      else
      {
        output = fopen (filename, "a");
        if (!output)
        {
          fprintf (stderr,
                   "WordMonitor::WordMonitor: cannot open %s for writing ",
                   filename);
          perror ("");
          output = stderr;
          return;
        }
      }
      if (fields.Count () > 1)
      {
        char *style = fields[1];
        if (!mystrcasecmp (style, "rrd"))
          output_style = WORD_MONITOR_RRD;
        else
          output_style = WORD_MONITOR_READABLE;
      }
    }
    TimerStart ();
  }
}

WordMonitor::~WordMonitor ()
{
  TimerStop ();
  if (output != stderr)
    fclose (output);
}

void
WordMonitor::Initialize (const Configuration & config_arg)
{
  if (instance != 0)
    delete instance;
  instance = new WordMonitor (config_arg);
}

const String
WordMonitor::Report () const
{
  String output;
  int i;
  time_t now = time (0);

  if (output_style == WORD_MONITOR_RRD)
    output << (int) now << ":";

  for (i = 0; i < WORD_MONITOR_VALUES_SIZE; i++)
  {
    if (!values_names[i])
      break;
    if (values_names[i][0])
    {
      if (output_style == WORD_MONITOR_READABLE)
      {
        output << values_names[i] << ": " << values[i];
        if ((now - elapsed) > 0)
        {
          output << ", per sec : " << (int) (values[i] / (now - started));
          output << ", delta : " << (values[i] - old_values[i]);
          output << ", per sec : " << (int) ((values[i] - old_values[i]) /
                                             (now - elapsed));
        }
        output << "|";
      }
      else if (output_style == WORD_MONITOR_RRD)
      {
        output << values[i] << ":";
      }
    }
  }
  memcpy ((char *) old_values, (char *) values,
          sizeof (unsigned int) * WORD_MONITOR_VALUES_SIZE);
  return output;
}

static void
handler_alarm (int signal)
{
  WordMonitor *monitor = WordMonitor::Instance ();
  if (!monitor)
  {
    fprintf (stderr, "WordMonitor::handler_alarm: no instance\n");
    return;
  }
  monitor->TimerClick (signal);
}

void
WordMonitor::TimerStart ()
{
  if (period < 5)
  {
    fprintf (stderr,
             "WordMonitor::TimerStart: wordlist_monitor_period must be > 5 (currently %d) otherwise monitoring is not accurate\n",
             period);
    return;
  }

#ifndef _MSC_VER                /* _WIN32 */
  struct sigaction action;
  struct sigaction old_action;
  memset ((char *) &action, '\0', sizeof (struct sigaction));
  memset ((char *) &old_action, '\0', sizeof (struct sigaction));
  action.sa_handler = handler_alarm;
  if (sigaction (SIGALRM, &action, &old_action) != 0)
  {
    fprintf (stderr, "WordMonitor::TimerStart: installing SIGALRM ");
    perror ("");
  }

  if (old_action.sa_handler != SIG_DFL)
  {
    fprintf (stderr,
             "WordMonitor::TimerStart: found an installed action while installing SIGALRM, restoring old action\n");
    if (sigaction (SIGALRM, &old_action, NULL) != 0)
    {
      fprintf (stderr, "WordMonitor::TimerStart: installing old SIGALRM ");
      perror ("");
    }
    return;
  }
#endif

  fprintf (output,
           "----------------- WordMonitor starting -------------------\n");
  if (output_style == WORD_MONITOR_RRD)
  {
    fprintf (output, "Started:%ld\n", started);
    fprintf (output, "Period:%d\n", period);
    fprintf (output, "Time:");
    int i;
    for (i = 0; i < WORD_MONITOR_VALUES_SIZE; i++)
    {
      if (!values_names[i])
        break;
      if (values_names[i][0])
        fprintf (output, "%s:", values_names[i]);
    }
    fprintf (output, "\n");
  }
  fflush (output);
  TimerClick (0);
}

void
WordMonitor::TimerClick (int signal)
{
  if (signal)
  {
    //
    // Do not report if less than <period> since last report.
    //
    if (time (0) - elapsed >= period)
    {
      fprintf (output, "%s\n", (const char *) Report ());
      elapsed = time (0);
      fflush (output);
    }
  }
#ifndef _MSC_VER                /* _WIN32 */
  alarm (period);
#endif
}

void
WordMonitor::TimerStop ()
{
  if (period > 0)
  {
#ifndef _MSC_VER                /* _WIN32 */
    alarm (0);
    struct sigaction action;
    memset ((char *) &action, '\0', sizeof (struct sigaction));
    action.sa_handler = SIG_DFL;
    if (sigaction (SIGALRM, &action, NULL) != 0)
    {
      fprintf (stderr,
               "WordMonitor::TimerStart: resetting SIGALRM to SIG_DFL ");
      perror ("");
    }

    // Make sure last report is at least one second older than the previous one.
    //
    if (time (0) - elapsed < 1)
      sleep (2);
    fprintf (output, "%s\n", (const char *) Report ());
    fprintf (output,
             "----------------- WordMonitor finished -------------------\n");
#endif
  }
}

//
// C interface to WordMonitor instance
//

extern "C"
{
  void word_monitor_click ()
  {
    WordMonitor *monitor = WordMonitor::Instance ();
#ifndef _MSC_VER                /* _WIN32 */
    if (monitor)
        monitor->TimerClick (SIGALRM);
#endif
  }
  void word_monitor_add (int index, unsigned int value)
  {
    WordMonitor *monitor = WordMonitor::Instance ();
    if (monitor)
      monitor->Add (index, value);
  }
  void word_monitor_set (int index, unsigned int value)
  {
    WordMonitor *monitor = WordMonitor::Instance ();
    if (monitor)
      monitor->Set (index, value);
  }
  unsigned int word_monitor_get (int index)
  {
    WordMonitor *monitor = WordMonitor::Instance ();
    if (monitor)
      return monitor->Get (index);
    else
      return 0;
  }
}
