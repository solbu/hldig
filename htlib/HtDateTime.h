///////
   //   Class for Date and Time
  //    Gabriele Bartolini - Prato - Italia
   //   Started: 22.04.1999
///////

// Version: 1.0
// Release date: 29.04.1999

// General purpose of HtDateTime
// 	 The general purpose of this class, is to provide an interface for
// 	 date and time managing, and to unload the programmer to manage
// 	 time_t, struct tm, time system functions and other related procedures
// 	 locally ... Everything regarding time and date must be put here.
// 	 D'you agree with me? Well, here is a summary of the class capabilities.

// Attributes of the class:
//
// 	 HtDateTime class has only 2 member attributes
// 	    -  time_t   Ht_t
// 	    -  bool  	 local_time
//
// 	    Obviously Ht_t contains the most important piece of information.
// 	    local_time assumes a true value if we wanna consider the date and
// 	    time information as local. False means that our object value is
// 	    referred to the Greenwich Meridian time.

// Interface provided:
//
// 	 Construction:
// 	    - Default: set the date time value to now
// 	    - By passing a time_t value or pointer: Set to it
// 	    - By passing a struct tm value or pointer: Set to it
// 	       The last one could be useful sometimes. But it had better not to
// 	       be used.
//
// 	 Parsing interface:
// 	    Not yet implemented ...  :-)
//
// 	 Setting Interface:
// 	    - from time_t: copy the time_t value into the object
// 	    - from struct tm: set the object's time_t value by converting
//  	       the value from the struct tm. If local_time is set to true,
// 	 	  converts it with mktime, else uses Httimegm.
// 	    - set to now
// 	    - from a string, by passing the input format: the method uses
// 	       strptime syntax (and invokes Htstrptime). For now, timezone
// 	       is ignored, and so data are stored as a GM date time value.
// 	    - from an int series, by specifying all the information (year,
// 	       month, day, hour, minute and second). It's all stored as
// 	       GM value.
// 	    - from various standard formats, such as C asctime, RFC 1123,
// 	       RFC 850 (these 3 types are suggested by the HTTP/1.1 standard),
// 	       ISO 8601, date and time default representation for the locale.
// 	       This list could get longer ... It all depends on us.
// 	    - setting the date and time to be represented in a local value
// 	       or universal (GM) one.
//
// 	 Getting Interface
// 	    - in a personalized output format, by passing a string with
// 	       strftime values.
// 	    - in various standard formats, like C asctime, RFC 1123,
// 	       RFC 850, ISO 8601 (short too), date and time default
// 	       representation for the locale.
// 	    - getting the time_t value
// 	    - queries the local time status
// 	    - getting specific piece of information of both the date and the
// 	       the time, like the year, the month, the day of the week, of
// 	       the year or of the month, ... In short, every kind of thing
// 	       a tm structure is able to store ...
//
// 	 Operator overloading
// 	    - Copy
// 	    - Every kind of logical comparison between 2 objects
//
// 	 Comparison interface
// 	    This is divided in 2 sections.
// 	       - Static section:
// 	       	comparison are made on a 2 struct tm values basis.
// 	       	 It's possible to compare the whole date time value, or 
// 	       	 simply the date or the time value.
// 	       - Member functions section:
//          	comparison are made between 2 HtDateTime objects.
// 	       	 You can compare either the whole date time, or the date, or the
// 	       	 time, both as they are or referring their values to the GM value.
//
// 	 System functions interface
// 	    They are all done with previous "configure" checks
// 	    - for strptime
// 	    - for timegm
//
// 	 Static methods
// 	    - check for a leap year
// 	    - check for a valid year number (according with time_t capabilities) 
// 	    - check for a valid month number
// 	    - check for a valid day
// 	    - converts a 2 digits year number into a 4 digits one: from 1970 to 2069.
// 	    - converts a 4 digits year number into a 2 digits one.
//
// 	 Test Interface (only by defining TEST_HTDATETIME directive).
//


#ifndef _HTDATETIME_H
#define _HTDATETIME_H

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <htString.h>

#ifdef HAVE_CONFIG
# include <htconfig.h>
#endif


// If you wanna do some tests
#define TEST_HTDATETIME


class HtDateTime
{
public:

///////
   //   Construction
///////

   // Default: now and local
   HtDateTime() {SettoNow(); ToLocalTime();}

   // From a time_t value and pointer
   HtDateTime(time_t &t) {SetDateTime(t); ToLocalTime();}
   HtDateTime(time_t *t) {SetDateTime(t); ToLocalTime();}
   
   // From a struct tm value and pointer
   HtDateTime(struct tm &t) {SetDateTime(t); ToLocalTime();}
   HtDateTime(struct tm *t) {SetDateTime(t); ToLocalTime();}


///////
   //   Interface methods
///////

///////
   //   "Parsing" interface
///////

   int Parse(const char *);	// It looks for the similar format
   	       	   	       	// then sets the date by invoking
						// right method
   

///////
   //   "Setting" interface
///////

   // Setting from a time_t value
   void SetDateTime(const time_t &t) { Ht_t =  t; }      	   // by reference
   void SetDateTime(const time_t *t) { Ht_t = *t; } 	        // by pointer

   // Set object time_t value from a struct tm
   inline void SetDateTime(struct tm *);  	   	       	   // by pointer
   inline void SetDateTime(struct tm &t) { SetDateTime(&t);}   // by reference
   
   // Set GM Time from single values input
   // Return true if it all went good, false else
   bool SetGMDateTime(	int year, int mon, int mday,
      	             	int hour=0, int min=0, int sec=0);
				
   // Set to Now
   void SettoNow() { SetDateTime ( (const time_t &) time(NULL) ); }

   // Parsing various input string format 
   // It ignores time Zone value - always stores as GM
   char *SetFTime(char *, const char *); // as strptime

   void SetAscTime(char *);	   	 // Sun Nov  6 08:49:37 1994
   void SetRFC1123(char *);	   	 // Sun, 06 Nov 1994 08:49:37 GMT
   void SetRFC850(char *);	   	 // Sunday, 06-Nov-94 08:49:37 GMT
   void SetISO8601(char *);     	 // 1994-11-06 08:49:37 GMT

   void SetDateTimeDefault(char *); // Default date and time representation
   	       	   	       	   	 // for the locale

   ///////
   	 //   Methods for setting Local and GM time formats (Switches)
   ///////

   void ToLocalTime() {local_time=true;}
   void ToGMTime() {local_time=false;}


///////
   //   "Getting" interface
///////


   ///////
      //   Output formats
   ///////

   // Personalized output
   char *GetFTime(const char *format) const;   	   	 // as strftime
   size_t GetFTime(char *, size_t, const char *) const; // as strftime

   char *GetAscTime() const;	   	 // Sun Nov  6 08:49:37 1994
   char *GetRFC1123() const;	   	 // Sun, 06 Nov 1994 08:49:37 GMT
   char *GetRFC850() const;	   	 // Sunday, 06-Nov-94 08:49:37 GMT
   char *GetISO8601() const;     	 // 1994-11-06 08:49:37 GMT

   char *GetDateTimeDefault() const;   // Default date and time representation
   	       	   	       	   	    // for the locale

   // Partial (only date or only time)
   char *GetShortISO8601() const;	 // 1994-11-06
   char *GetDateDefault() const; 	 // Default date form for the locale
   char *GetTimeDefault() const; 	 // Default time form for the locale


   ///////
      //   Gets the time_t value
   ///////

   time_t GetTime_t() const {return Ht_t;}


   ///////
      //   Gets specific date and time values (from a struct tm)
   ///////

   // Gets the year
   int GetYear() const  { return ( GetStructTM().tm_year + 1900) ;}
   
   // Gets the month
   int GetMonth() const { return (GetStructTM().tm_mon + 1);}

   // Gets the day of the week (since Sunday)
   int GetWDay() const { return (GetStructTM().tm_wday + 1);}

   // Gets the day of the month
   int GetMDay() const { return GetStructTM().tm_mday;}

   // Gets the day since january 1
   int GetYDay() const { return (GetStructTM().tm_yday + 1);}
   
   // Gets the hour
   int GetHour() const { return GetStructTM().tm_hour;}
   
   // Gets the minute
   int GetMinute() const { return GetStructTM().tm_min;}
   
   // Gets the second
   int GetSecond() const { return GetStructTM().tm_sec;}
   
   // Daylight saving time is in effect at that time?
   int GetIsDst() const { return GetStructTM().tm_isdst;}


   ///////
      //   Methods for querying localtime status
   ///////

   bool isLocalTime() const {return local_time;}
   bool isGMTime() const {return !local_time;}



///////
   //   Methods for comparison
///////

   // Returns 0 if equal, -1 if tm1 is lower than tm2, 1 if tm1 is greater than tm2

   int DateTimeCompare (const HtDateTime &) const; // Compares both date and time

   int DateCompare (const HtDateTime &) const;  	 // Compares the date
   int TimeCompare (const HtDateTime &) const;  	 // Compares the time

   // Refers the date and the time to a GM value, then compares
   int GMDateTimeCompare (const HtDateTime &) const; // Compares both date and time
   int GMDateCompare (const HtDateTime &) const;   // Compares the date
   int GMTimeCompare (const HtDateTime &) const;   // Compares the time


///////
   //   Operator overloading
///////

   // For comparisons

   inline bool operator==(const HtDateTime &right) const;
   inline bool operator<(const HtDateTime &right) const;

   bool operator!=(const HtDateTime &right) const
      	 {return !( *this == right );}

   bool operator>=(const HtDateTime &right) const
      	 {return !( *this < right);}

   bool operator<=(const HtDateTime &right) const
      	 {return !( right < *this);}

   bool operator>(const HtDateTime &right) const
      	 {return right < *this; }


   // For Copy

   inline HtDateTime &operator=(const HtDateTime &right);




///////  //   STATIC METHODS   //  ///////

   // Here we can add static methods as we want more  :-)
   // Then invoke them with HtDateTime::MethodXXX ()
   

   inline static bool LeapYear(int);	  	   	 // Is a leap year?
   
   // These checks are made for time_t compatibility
   inline static bool isAValidYear(int);	   	 // Is a valid year number
   
   inline static bool isAValidMonth(int);	   	 // Is a valid month number
   inline static bool isAValidDay(int, int, int);  // Is a valid day
   

   // Converts a 2 digits year in a 4 one - with no checks
   static int Year_From2To4digits (int y)
   {
   	 if ( y >= 70 ) return y+1900;
   	 else return y+2000;
   }

   // Converts a 4 digits year in a 2 one - with no checks
   static int Year_From4To2digits (int y)
   {
   	 if ( y >= 2000 ) return y - 2000;
   	 else return y - 1900;
   }


   // Check equality from 2 struct tm pointers
   // Returns 0 if equal, -1 if tm1 is lower than tm2, 1 if tm1 is greater than tm2

   // Compares the whole time information (both date and time)
   static int DateTimeCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);

   // Compares only date
   static int DateCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);

   // Compares only time
   static int TimeCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);



///////  //   HIDDEN ATTRIBUTES & METHODS   //  ///////

protected: 		// to permit inheritance

   time_t Ht_t;
   bool local_time;

   static const int days[];

///////
   //   Sets and gets the struct tm depending on local_time status
///////

   void RefreshStructTM() const; 	       	// Refresh its content
   struct tm &GetStructTM() const;        	// gets it
   void GetStructTM(struct tm & t) const { t=GetStructTM(); } // Gets and copy


///////
   //   Gets the struct tm ignoring local_time status
///////

   struct tm &GetGMStructTM() const;        	// gets it
   void GetGMStructTM(struct tm &) const; 	// Gets and copy


///////
   //   Interface for system functions
///////

   // Interface for strptime
   static char * Htstrptime (char *buf, char *format,
   	       	   	       	struct tm *tm);
   // Interface for timegm
   static time_t Httimegm (struct tm*);
   

#ifdef TEST_HTDATETIME

///////
   //   Only for debug: view of struct tm fields
///////

public:

   static void ViewStructTM(struct tm *); 	// view of struct tm fields
   virtual void ViewStructTM();   	  	  	// view of struct tm fields
   void ViewFormats();     	   	       	// View of various formats

   void ComparisonTest (const HtDateTime &) const; // comparison
   

   // Test of the class
   static int Test(void);
   static int Test(char **test_dates, const char *format);
   
#endif

};


#endif
