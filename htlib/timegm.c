//
// timegm.cc
//
// timegm: Portable version of timegm (mytimegm) for ht://Dig
//         Based on a version from the GNU C Library
//         and a previous implementation for ht://Dig
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: timegm.c,v 1.14 1999/09/11 05:03:52 ghutchis Exp $
//

/* Copyright (C) 1993, 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Eggert (eggert@twinsun.com).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


/* #define TEST_TIMEGM */

#include <time.h>
#ifdef TEST_TIMEGM
#include <stdio.h>
#include <stdlib.h>
#endif

static struct tm *my_mktime_gmtime_r (const time_t *t, struct tm *tp);

static struct tm *my_mktime_gmtime_r (const time_t *t, struct tm *tp)
{
  struct tm *l = gmtime (t);
  if (! l)
    return 0;
  *tp = *l;
  return tp;
}

time_t Httimegm(tmp)
struct tm *tmp;
{
  static time_t gmtime_offset;
  tmp->tm_isdst = 0;
  return __mktime_internal (tmp, my_mktime_gmtime_r, &gmtime_offset);
}

#ifdef TEST_TIMEGM

void parse_time(char *s, struct tm *tm)
{
  sscanf(s, "%d.%d.%d %d:%d:%d", 
	 &tm->tm_year, &tm->tm_mon, &tm->tm_mday,
	 &tm->tm_hour, &tm->tm_min, &tm->tm_sec);
  tm->tm_year -= 1900;
  tm->tm_mon--;
}

void print_time(struct tm *tm)
{
  fprintf(stderr, "%04d.%02d.%02d %02d:%02d:%02d",
	  tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	  tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int time_equal(struct tm *tm1, struct tm *tm2)
{
  return ((tm1->tm_year == tm2->tm_year) &&
	  (tm1->tm_mon == tm2->tm_mon) &&
	  (tm1->tm_mday == tm2->tm_mday) &&
	  (tm1->tm_hour == tm2->tm_hour) && 
	  (tm1->tm_min == tm2->tm_min) &&
	  (tm1->tm_sec == tm2->tm_sec));
}

int main(void)
{
  char *test_dates[] = 
  {
    "1970.01.01 00:00:00",
    "1970.01.01 00:00:01",
    "1972.02.05 23:59:59",
    "1972.02.28 00:59:59",
    "1972.02.28 23:59:59",
    "1972.02.29 00:00:00",
    "1972.03.01 13:00:04",
    "1973.03.01 12:00:00",
    "1980.01.01 00:00:05",
    "1984.12.31 23:00:00",
    "1997.06.05 17:55:35",
    "1999.12.31 23:00:00",
    "2000.01.01 00:00:05",
    "2000.02.28 23:00:05",
    "2000.02.29 23:00:05",
    "2000.03.01 00:00:05",
    "2007.06.05 17:55:35",
    "2038.01.19 03:14:07",
    0
  };
  int i, ok = 1;
  struct tm orig, *conv;
  time_t t;

  for (i = 0; (test_dates[i]); i++)
  {
    parse_time(test_dates[i], &orig);
    t = Httimegm(&orig);
    conv = gmtime(&t);
    if (!time_equal(&orig, conv))
    {
      fprintf(stderr, "timegm() test failed!\n  Original: ");
      print_time(&orig);
      fprintf(stderr, "\n  Converted: ");
      print_time(conv);
      fprintf(stderr, "\n  time_t: %ld\n", (long) t);
      ok = 0;
    }
  }
  exit(ok ? 0 : 1);
}

#endif
