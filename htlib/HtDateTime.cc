#include <stdlib.h>
#include <HtDateTime.h>
#include <string.h>
#include <iostream.h>


///////
   //    Static local variable : Visible only here !!!
///////

#define MAXSTRTIME 256  // Max length of _strtime

static struct tm Ht_tm;
static char _strtime[MAXSTRTIME];


///////
  //     Recognized Date Formats
///////

//     RFC1123: Sun, 06 Nov 1994 08:49:37 GMT
#define RFC1123_FORMAT "%a, %d %b %Y %H:%M:%S %Z"

//     RFC850 : Sunday, 06-Nov-94 08:49:37 GMT
#define RFC850_FORMAT  "%A, %d-%b-%y %H:%M:%S %Z"

//     ANSI C's asctime() format : Sun Nov  6 08:49:37 1994
#define ASCTIME_FORMAT  "%a %b %e %H:%M:%S %Y"

// 	  ISO8601 : 1994-11-06 08:49:37 GMT
#define ISO8601_FORMAT "%Y-%m-%d %H:%M:%S %Z"

// 	  ISO8601 (short version): 1994-11-06
#define ISO8601_SHORT_FORMAT "%Y-%m-%d"



///////
   //   Initialization
///////

const int HtDateTime::days[] = {	31, 28, 31, 30, 31, 30,
				31, 31, 30, 31, 30, 31};


///////   //   Input Formats   //   ///////

///////
   //   Personalized format such as C strftime function
  //     Overloaded version 1
   //    It ignores, for now, Time Zone values
///////

char *HtDateTime::SetFTime(char *buf, const char *format)
{

   register char *p;

   ToGMTime();    // This must be set cos strptime always stores in GM 

   p = (char *) Htstrptime (buf, (char *) format, & Ht_tm);

#ifdef TEST_HTDATETIME
//   ViewStructTM(& Ht_tm);
#endif
   
   // Assign the new value to time_t value
   SetDateTime(Ht_tm);

   return p;
   
}


///////
   //   C asctime() standard format
///////

void HtDateTime::SetAscTime(char *s)
{

   SetFTime(s, ASCTIME_FORMAT);

}

///////
   //   RFC1123 standard Date format
 //     Sun, 06 Nov 1994 08:49:37 GMT
///////

void HtDateTime::SetRFC1123(char *s)
{

   // abbreviated weekday name;
   // day of the month;
   // abbreviated month name;
   // year as ccyy;
   // hour ( 00 - 23);
   // minute ( 00 - 59);
   // seconds ( 00 - 59);
   // time zone name;

   SetFTime(s, RFC1123_FORMAT);

}


///////
   //   RFC850 standard Date format
 //     Sunday, 06-Nov-1994 08:49:37 GMT
///////

void HtDateTime::SetRFC850(char *s)
{

   // weekday name;
   // day of the month;
   // abbreviated month name;
   // year within century;
   // hour ( 00 - 23);
   // minute ( 00 - 59);
   // seconds ( 00 - 59);
   // time zone name;

   SetFTime(s, RFC850_FORMAT);

}


///////
   //   ISO8601 standard Date format
 //     1994-11-06 08:49:37 GMT
///////

void HtDateTime::SetISO8601(char *s)
{

   // year as ccyy;
   // month ( 01 - 12)
   // day of the month
   // hour ( 00 - 23)
   // minute ( 00 - 59)
   // seconds ( 00 - 59);
   // time zone name;

   SetFTime(s, ISO8601_FORMAT);
   
}


///////
   //   Default date and time format for the locale
///////

void HtDateTime::SetDateTimeDefault(char *s)
{

   SetFTime(s, "%c");
   
}




///////   //   Output Formats   //   ///////


///////
   //   Personalized format such as C strftime function
  //     Overloaded version 1
///////

size_t HtDateTime::GetFTime(char *s, size_t max, const char *format) const
{
   // Refresh static struct tm variable

   RefreshStructTM();

   return strftime(s, max, format, & Ht_tm);

}

///////
   //   Personalized format such as C strftime function
  //     Overloaded version 2 - The best to be used outside
   //       	   	       	  for temporary uses
///////

char *HtDateTime::GetFTime(const char *format) const
{

   // Invoke GetFTime overloaded method
   
   if(GetFTime(_strtime, MAXSTRTIME, format))
      return (char *)_strtime;
   else return NULL;

}

///////
   //   RFC1123 standard Date format
 //     Sun, 06 Nov 1994 08:49:37 GMT
///////

char *HtDateTime::GetRFC1123() const
{

   // abbreviated weekday name;
   // day of the month;
   // abbreviated month name;
   // year as ccyy;
   // hour ( 00 - 23);
   // minute ( 00 - 59);
   // seconds ( 00 - 59);
   // time zone name;

   GetFTime(_strtime, MAXSTRTIME, RFC1123_FORMAT);

   return (char *)_strtime;
   
}

///////
   //   RFC850 standard Date format
 //     Sunday, 06-Nov-94 08:49:37 GMT
///////

char *HtDateTime::GetRFC850() const
{

   // full weekday name
   // day of the month
   // abbreviated month name
   // year within century ( 00 - 99 )
   // hour ( 00 - 23)
   // minute ( 00 - 59)
   // seconds ( 00 - 59);
   // time zone name;

   GetFTime(_strtime, MAXSTRTIME, RFC850_FORMAT);

   return (char *)_strtime;
   
}

///////
   //   C asctime() standard format
///////

char *HtDateTime::GetAscTime() const
{

   GetFTime(_strtime, MAXSTRTIME, ASCTIME_FORMAT);
   return (char *)_strtime;

}

///////
   //   ISO8601 standard Date format
 //     1994-11-06 08:49:37 GMT
///////

char *HtDateTime::GetISO8601() const
{

   // year as ccyy;
   // month ( 01 - 12)
   // day of the month
   // hour ( 00 - 23)
   // minute ( 00 - 59)
   // seconds ( 00 - 59);
   // time zone name;

   GetFTime(_strtime, MAXSTRTIME, ISO8601_FORMAT);

   return (char *)_strtime;
   
}

///////
   //   ISO8601 standard Date format
 //     1994-11-06 08:49:37 GMT
///////

char *HtDateTime::GetShortISO8601() const
{

   // year as ccyy;
   // month ( 01 - 12)
   // day of the month

   GetFTime(_strtime, MAXSTRTIME, ISO8601_SHORT_FORMAT);

   return (char *)_strtime;
   
}

///////
   //   Default date and time format for the locale
///////

char *HtDateTime::GetDateTimeDefault() const
{

   GetFTime(_strtime, MAXSTRTIME, "%c");

   return (char *)_strtime;
   
}

///////
   //   Default date format for the locale
///////

char *HtDateTime::GetDateDefault() const
{

   GetFTime(_strtime, MAXSTRTIME, "%x");

   return (char *)_strtime;
   
}

///////
   //   Default time format for the locale
///////

char *HtDateTime::GetTimeDefault() const
{

   GetFTime(_strtime, MAXSTRTIME, "%X");

   return (char *)_strtime;
   
}



///////
   //   Set the static struct tm depending on localtime status
///////


void HtDateTime::RefreshStructTM() const
{

   if(local_time)
      // Setting localtime
   	 memcpy(& Ht_tm, localtime(&Ht_t), sizeof(struct tm));
   else
   	 // Setting UTC or GM time
   	 memcpy(& Ht_tm , gmtime(&Ht_t), sizeof(struct tm));
	 
}


void HtDateTime::SetDateTime(struct tm *ptm)
{

   if(local_time)
   	 Ht_t = mktime(ptm);  	// Invoke mktime
   else
   	 Ht_t = Httimegm(ptm);	// Invoke timegm alike function
   
}


// Sets date by passing specific values
// The values are reffered to the GM date time
// Return false if failed

bool HtDateTime::SetGMDateTime ( int year, int mon, int mday,
			int hour, int min, int sec)
{
   struct tm tm_tmp;

   // Year
   
   if ( ! isAValidYear (year) ) return false;

   if( year < 100)
	 year=Year_From2To4digits (year); // For further checks it's converted

   // Assigning the year
   
   tm_tmp.tm_year=year-1900;


   // Month

   if( ! isAValidMonth(mon) ) return false;

   tm_tmp.tm_mon=mon-1; // Assigning the month to the structure


   // Day

   if ( ! isAValidDay ( mday, mon, year ) ) return false;
   
   tm_tmp.tm_mday=mday; // Assigning the day of the month



   if(hour >= 0 && hour < 24) tm_tmp.tm_hour = hour;
   	 else return false;	
	
   if(min >= 0 && min < 60) tm_tmp.tm_min = min;
   	 else return false;	
	
   if(sec >= 0 && sec < 60) tm_tmp.tm_sec = sec;
   	 else return false;	

   tm_tmp.tm_yday = 0; // day of the year (to be ignored)
   tm_tmp.tm_isdst = 0; // default for GM (to be ignored)

   // Now we are going to insert the new values as time_t value
   // This can only be done using GM Time and so ...

   if (isLocalTime())
   {
   	 ToGMTime(); 	       // Change to GM Time
   	 SetDateTime(&tm_tmp); // commit it
	 ToLocalTime();       // And then return to Local Time
   }
   else SetDateTime(&tm_tmp); // only commit it

   return true;
   
}


///////
   //   Gets a struct tm from the value stored in the object
  //    It's a protected method. Not visible outside the class 
///////

struct tm &HtDateTime::GetStructTM() const 
{
	RefreshStructTM(); // refresh it

	return Ht_tm;
}


struct tm &HtDateTime::GetGMStructTM() const 
{
   GetGMStructTM (Ht_tm);
   return Ht_tm;
}

void HtDateTime::GetGMStructTM(struct tm & t) const
{
   // Directly gets gmtime value
   memcpy(& t , gmtime(& Ht_t), sizeof(struct tm));
}


///////
   //   Is a leap year?
///////

bool HtDateTime::LeapYear (int y)
{

   if(y % 400 == 0 || ( y % 100 != 0 && y % 4 == 0))
   	 return true; 	// a leap year
   else
   	 return false;	// and not
}


///////
   //   Is a valid year number?
///////

bool HtDateTime::isAValidYear (int y)
{

   if(y >= 1970 && y < 2069) return true; // simple check and most likely

   if(y >= 0 && y < 100) return true;  // 2 digits year number
   
   return false;
   
}


///////
   //   Is a valid month number?
///////

bool HtDateTime::isAValidMonth (int m)
{

   if( m >= 1 && m <= 12) return true;
   else return false;
   
}


///////
   //   Is a valid day?
///////

bool HtDateTime::isAValidDay (int d, int m, int y)
{

   if ( ! isAValidYear (y) ) return false;	// Checks for the year

   if ( ! isAValidMonth (m) ) return false;	// Checks for the month
   
   if(m == 2)
   {
   
   	 // Expands the 2 digits year number
   	 if ( y < 100 ) y=Year_From2To4digits(y);
	 
	 if ( LeapYear (y) ) // Checks for the leap year
      {
	    if (d >= 1 && d <= 29) return true;
	    else return false;
      }
   }

   // Acts as default
   
   if (d >= 1 && d <= days [m -1]) return true;
   else return false;
   
}



///////
   //   Operator overloading
///////

bool HtDateTime::operator==(const HtDateTime &right) const
{
	if(Ht_t==right.Ht_t)
		return true;
	else
		return false;
}

bool HtDateTime::operator<(const HtDateTime &right) const
{
	if(Ht_t < right.Ht_t) return true;
	else return false;
}


///////
   //   Copy
///////

HtDateTime &HtDateTime::operator=(const HtDateTime &right)
{
	Ht_t=right.Ht_t;      	   	 // Copy the time_t value
	local_time=right.local_time;	 // Copy the local_time flag
	
	return *this;
}


///////
   //   Comparison methods
///////


int HtDateTime::DateTimeCompare (const HtDateTime & right) const
{
   int result;
   
   // Let's compare the date

   result=DateCompare(right);
   
   if(result) return result;
   
   // Same date. Let's compare the time

   result=TimeCompare(right);

   return result; // Nothing more to check

}


int HtDateTime::GMDateTimeCompare (const HtDateTime & right) const
{
	// We must compare the whole time_t value
	
	if ( * this > right) return 1;	 // 1st greater than 2nd
	if ( * this < right) return 1;	 // 1st lower than 2nd
	
	return 0;
	
}


int HtDateTime::DateCompare (const HtDateTime & right) const
{

   // We must transform them in 2 struct tm variables
	
   struct tm tm1, tm2;
   
   this->GetGMStructTM (tm1);
   right.GetGMStructTM (tm2);

   // Let's compare them
   return DateCompare (&tm1, &tm2);
	
}


int HtDateTime::GMDateCompare (const HtDateTime & right) const
{

   // We must transform them in 2 struct tm variables
   // both referred to GM time
	
   struct tm tm1, tm2;
   
   this->GetGMStructTM (tm1);
   right.GetGMStructTM (tm2);

   // Let's compare them
   return DateCompare (&tm1, &tm2);
	
}


int HtDateTime::TimeCompare (const HtDateTime & right) const
{

   // We must transform them in 2 struct tm variables
	
   struct tm tm1, tm2;
	
   this->GetStructTM (tm1);
   right.GetStructTM (tm2);

   return TimeCompare (&tm1, &tm2);
	
}


int HtDateTime::GMTimeCompare (const HtDateTime & right) const
{

   // We must transform them in 2 struct tm variables
	
   struct tm tm1, tm2;

   // We take the GM value of the time	
   this->GetGMStructTM (tm1);
   right.GetGMStructTM (tm2);

   return TimeCompare (&tm1, &tm2);
	
}



///////
   //   Static methods of comparison between 2 struct tm pointers
///////


///////
   //   Compares only the date (ignoring the time)
///////

int HtDateTime::DateCompare(const struct tm *tm1, const struct tm *tm2)
{

   // Let's check the year
   
   if (tm1->tm_year < tm2->tm_year) return -1;
   if (tm1->tm_year > tm2->tm_year) return  1;

   // Same year. Let's check the month
   if (tm1->tm_mon < tm2->tm_mon) return -1;
   if (tm1->tm_mon > tm2->tm_mon) return 1;
   
   // Same month. Let's check the day of the month
   
   if (tm1->tm_mday < tm2->tm_mday) return -1;
   if (tm1->tm_mday > tm2->tm_mday) return 1;

   // They are equal for the date
   return 0;
}


///////
   //   Compares only the time (ignoring the date)
///////

int HtDateTime::TimeCompare(const struct tm *tm1, const struct tm *tm2)
{

   // Let's check the hour
   
   if (tm1->tm_hour < tm2->tm_hour) return -1;
   if (tm1->tm_hour > tm2->tm_hour) return  1;

   // Same hour . Let's check the minutes
   
   if (tm1->tm_min < tm2->tm_min) return -1;
   if (tm1->tm_min > tm2->tm_min) return 1;

   // Ooops !!! Same minute. Let's check the seconds
   
   if (tm1->tm_sec < tm2->tm_sec) return -1;
   if (tm1->tm_sec > tm2->tm_sec) return 1;

   // They are equal for the time
   return 0;
}


///////
   //   Compares both date and time
///////

int HtDateTime::DateTimeCompare(const struct tm *tm1, const struct tm *tm2)
{

   int compare_date = DateCompare(tm1, tm2);
   
   if(compare_date) return compare_date;  // Different days

   // We are in the same day. Let's check the time

   int compare_time = TimeCompare(tm1, tm2);
   if(compare_time) return compare_time;  // Different time
   
   // Equal
   return 0;
}

char * HtDateTime::Htstrptime (  char *buf,
   	       	   	       	   char *format,
   	       	   	       	   struct tm *tm)
{

#if HAVE_STRPTIME
   return (char *) ::strptime (buf, format, tm);
#else
   return (char *) ::Htstrptime (buf, format, tm); // Has it to be changed ?
#endif
   
}


time_t HtDateTime::Httimegm (struct tm *tm)
{

#if HAVE_TIMEGM
   return ::timegm (tm);
#else
   // return ::Httimegm (tm); // To be changed if managed here
   static time_t gmtime_offset;
   tm->tm_isdst = 0;
   return __mktime_internal (tm, gmtime, &gmtime_offset);
}

#endif
   
}





///////
   //   Only for test and debug
///////

#ifdef TEST_HTDATETIME


///////
   //   View of struct tm fields
///////

void HtDateTime::ViewStructTM()
{
   // Default viewing: refresh depending on time_t value

   RefreshStructTM(); // Refresh static variable

   ViewStructTM(&Ht_tm);
}

void HtDateTime::ViewStructTM(struct tm *ptm)
{

	cout << "Struct TM fields" << endl;
	cout << "================" << endl;
	cout << "tm_sec   :\t" << ptm->tm_sec << endl;
	cout << "tm_min   :\t" << ptm->tm_min  << endl;
	cout << "tm_hour  :\t" << ptm->tm_hour << endl;
	cout << "tm_mday  :\t" << ptm->tm_mday << endl;
	cout << "tm_mon   :\t" << ptm->tm_mon  << endl;
	cout << "tm_year  :\t" << ptm->tm_year << endl;
	cout << "tm_wday  :\t" << ptm->tm_wday << endl;
	cout << "tm_yday  :\t" << ptm->tm_yday << endl;
	cout << "tm_isdst :\t" << ptm->tm_isdst<< endl;

}




int HtDateTime::Test(void)
{

   int ok=1;
   
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

   char *test_dates_ISO8601[] = 
   {
    "1970-01-01 00:00:00 GMT",
    "1970-01-01 00:00:00 CET",
    "1990-02-27 23:30:20 GMT",
    "1999-02-28 06:53:40 GMT",
    "1975-04-27 06:53:40 CET",
    0
   };

   char *test_dates_RFC1123[] = 
   {
   "Sun, 06 Nov 1994 08:49:37 GMT",
   "Sun, 25 Apr 1999 17:49:37 GMT",
   "Sun, 25 Apr 1999 17:49:37 CET",
    0
   };
  
   char *test_dates_RFC850[] = 
   {
   "Sunday, 06-Nov-94 08:49:37 GMT",
   "Sunday, 25-Apr-99 17:49:37 GMT",
   "Sunday, 25-Apr-99 17:49:37 CET",
    0
   };
  

   char myformat[]="%Y.%m.%d %H:%M:%S";

   // Tests a personal format  
   
   cout << endl << "Beginning Test of a personal format such as "
   	       	   << myformat << endl << endl;
   
   if (Test((char **)test_dates, (const char *)myformat))
   	 cout << "Test OK." << endl;
   else
   {
   	 cout << "Test Failed." << endl;
	 ok=0;
   }


   // Tests ISO 8601 Format

   cout << endl << "Beginning Test of ISO 8601 format" << endl << endl;

   if(Test((char **)test_dates_ISO8601, (const char *)ISO8601_FORMAT))
   	 cout << "Test OK." << endl;
   else
   {
   	 cout << "Test Failed." << endl;
	 ok=0;
   }


   // Tests RFC 1123 Format

   cout << endl << "Beginning Test of RFC 1123 format" << endl << endl;

   if (Test((char **)test_dates_RFC1123, (const char *)RFC1123_FORMAT))
   	 cout << "Test OK." << endl;
   else
   {
   	 cout << "Test Failed." << endl;
	 ok=0;
   }


   // Tests RFC 850 Format

   cout << endl << "Beginning Test of RFC 850 format" << endl << endl;

   if (Test((char **)test_dates_RFC850, (const char *)RFC850_FORMAT))
   	 cout << "Test OK." << endl;
   else
   {
   	 cout << "Test Failed." << endl;
	 ok=0;
   }


   return(ok ? 1 : 0);
  
}



int HtDateTime::Test(char **test_dates, const char *format)
{
  int i, ok = 1;
  HtDateTime orig, conv;

  for (i = 0; (test_dates[i]); i++)
  {
  
      cout << "\t " << i+1 << "\tDate string parsing of:" << endl;
	 cout << "\t\t" << test_dates[i] << endl;
	 cout << "\t\tusing format: " << format << endl << endl;

   	 orig.SetFTime(test_dates[i], format);
   
   	 orig.ComparisonTest(conv);
	 
      conv=orig;
   
      if (orig != conv)
   	 {
   	    cout << "HtDateTime test failed!" << endl;
	    cout << "\t Original : " << orig.GetRFC1123() << endl;
	    cout << "\t Converted: " << orig.GetRFC1123() << endl;
         ok = 0;
   	 }
   	 else
   	 {
	    orig.ToLocalTime();
	    cout << endl << "\t   Localtime viewing" << endl;
	    orig.ViewFormats();
	    orig.ToGMTime();
	    cout << endl << "\t   GMtime viewing" << endl;
	    orig.ViewFormats();
   	    //orig.ViewStructTM();
      }
	 
	 cout << endl;
	 
   }
   
   return ok;
}


void HtDateTime::ComparisonTest (const HtDateTime &right) const
{
   int result;
   
   
   cout << "Comparison between:" << endl;
   
   cout << " 1. " << this->GetRFC1123() << endl;
   cout << " 2. " << right.GetRFC1123() << endl;
   cout << endl;


///////
   //   Complete comparison
///////
   
      cout << "\tComplete comparison (date and time)" << endl;
      result = this->DateTimeCompare (right);

   cout << "\t\t " << this->GetDateTimeDefault();
   
   if (result > 0 )
   	 cout << " is greater than ";
   else if (result < 0 )
   	 cout << " is lower than ";
	 else cout << " is equal to ";

   cout << " " << right.GetDateTimeDefault() << endl;



///////
   //   Date comparison
///////
   
      cout << "\tDate comparison (ignoring time)" << endl;
      result = this->DateCompare (right);

   cout << "\t\t " << this->GetDateDefault();
   
   if (result > 0 )
   	 cout << " is greater than ";
   else if (result < 0 )
   	 cout << " is lower than ";
	 else cout << " is equal to ";

   cout << " " << right.GetDateDefault() << endl;


///////
   //   Date comparison (after GM time conversion)
///////
   
      cout << "\tDate comparison (ignoring time) - GM time conversion" << endl;
      result = this->GMDateCompare (right);

   cout << "\t\t " << this->GetDateDefault();
   
   if (result > 0 )
   	 cout << " is greater than ";
   else if (result < 0 )
   	 cout << " is lower than ";
	 else cout << " is equal to ";

   cout << " " << right.GetDateDefault() << endl;


	 
///////
   //   Time comparison
///////
   
      cout << "\tTime comparison (ignoring date)" << endl;
      result = this->TimeCompare (right);

   cout << "\t\t " << this->GetTimeDefault();
   
   if (result > 0 )
   	 cout << " is greater than ";
   else if (result < 0 )
   	 cout << " is lower than ";
	 else cout << " is equal to ";

   cout << " " << right.GetTimeDefault() << endl;


///////
   //   Time comparison (after GM time conversion)
///////
   
      cout << "\tTime comparison (ignoring date) - GM time conversion" << endl;
      result = this->GMTimeCompare (right);

   cout << "\t\t " << this->GetTimeDefault();
   
   if (result > 0 )
   	 cout << " is greater than ";
   else if (result < 0 )
   	 cout << " is lower than ";
	 else cout << " is equal to ";

   cout << " " << right.GetTimeDefault() << endl;
   
}



void HtDateTime::ViewFormats()
{

   cout << "\t\t RFC 1123 Format : " << GetRFC1123() << endl;
   cout << "\t\t RFC 850 Format  : " << GetRFC850() << endl;
   cout << "\t\t C Asctime Format: " << GetAscTime() << endl;
   cout << "\t\t ISO 8601 Format : " << GetISO8601() << endl;

}

#endif
