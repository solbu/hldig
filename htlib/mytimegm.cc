/*
 * Hopefully portable implementation of timegm().
 * Public domain, written by Pasi Eronen <pe@iki.fi>.
 *
 * Notes:
 * - DOESN'T MODIFY THE STRUCT TM!
 * - Epoch doesn't have to be at January 1, 1970.
 * - Out-of-range fields for struct tm are not handled properly.
 *
 */

/* #define TEST_TIMEGM */

#include <time.h>
#ifdef TEST_TIMEGM
#include <stdio.h>
#include <stdlib.h>
#endif

time_t mytimegm(struct tm *tm)
{
  #define LEAP_YEAR(y) ((((y+1900))%4 == 0) && (((y+1900))%100 != 0 || ((y+1900))%400 == 0))

  static struct tm epoch;
  static int epoch_initialized = 0;
  static int months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int y, days, m, secs;

  if (!epoch_initialized)
  {
    time_t zero = 0;
    struct tm *tmp = gmtime(&zero);
    epoch = *tmp;
    epoch_initialized = 1;
  }
  
  days = 0;
  if (tm->tm_year < epoch.tm_year)
    return -1;
  for (y = epoch.tm_year; (y < tm->tm_year); y++)
    days += LEAP_YEAR(y) ? 366 : 365;

  for (m = 0; (m < epoch.tm_mon); m++)
    days -= months[m];
  if (LEAP_YEAR(epoch.tm_year) && (epoch.tm_mon > 1))
    days--;
  for (m = 0; (m < tm->tm_mon); m++)
    days += months[m];
  if (LEAP_YEAR(tm->tm_year) && (tm->tm_mon > 1))
    days++;

  days += (tm->tm_mday - epoch.tm_mday);
  if (days < 0)
    return -1;
  
  secs = ((tm->tm_hour-epoch.tm_hour)*60+
	  (tm->tm_min-epoch.tm_min))*60 + 
         (tm->tm_sec-epoch.tm_sec);
  if ((days == 0) && (secs < 0))
    return -1;
  
  return days*24*60*60 + secs;

  #undef LEAP_YEAR
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

void main(void)
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
    "2038.01.19 03:14:07", /* last UTC second supported by signed 32-bit time_t */
    0
  };
  int i, ok = 1;
  struct tm orig, *conv;
  time_t t;

  for (i = 0; (test_dates[i]); i++)
  {
    parse_time(test_dates[i], &orig);
    t = mytimegm(&orig);
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
