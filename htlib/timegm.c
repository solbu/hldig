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
#include "mktime.c"
#ifdef TEST_TIMEGM
#include <stdio.h>
#include <stdlib.h>
#endif

time_t mytimegm(tmp)
struct tm *tmp;
{
  static time_t gmtime_offset;
  tmp->tm_isdst = 0;
  return __mktime_internal (tmp, __gmtime_r, &gmtime_offset);
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
