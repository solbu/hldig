///////
   //   Class for Date and Time
  //    Gabriele Bartolini - U.O. Rete Civica Comune di Prato - Italia
   //   Started: 22.04.1999
///////

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
   HtDateTime(time_t &t) {SetDateTime(t);}
   HtDateTime(time_t *t) {SetDateTime(t);}
   
   // From a struct tm value and pointer
   HtDateTime(struct tm &t) {SetDateTime(t);}
   HtDateTime(struct tm *t) {SetDateTime(t);}


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


	
///////
   //   Copy
///////

   inline HtDateTime &operator=(const HtDateTime &right);


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

   // Partial (only date or only time)
   char *GetShortISO8601() const;	 // 1994-11-06
   char *GetDateDefault() const; 	 // Default date form for the locale
   char *GetTimeDefault() const; 	 // Default time form for the locale


///////
   //   Gets the time_t value
///////

   time_t GetTime_t() {return Ht_t;}


///////
   //   Methods for setting Local and GM time formats (Switches)
///////

   void ToLocalTime() {local_time=true;}
   void ToGMTime() {local_time=false;}


///////
   //   Methods for querying localtime status
///////

   bool isLocalTime() {return local_time;}
   bool isGMTime() {return !local_time;}



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
   // Returns 0 if equal
   // -1 if tm1 is less than tm2
   // 1 else


   // Compares the whole time information (both date and time)
   static int DateTimeCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);

   // Compares only date
   static int DateCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);

   // Compares only time
   static int TimeCompare(const struct tm *tm1,
   	       	   	       const struct tm *tm2);


   // Interface for strptime
   static char * Htstrptime (char *buf, char *format,
   	       	   	       	struct tm *tm);
								  
   static time_t Httimegm (struct tm*);


///////  //   HIDDEN ATTRIBUTES & METHODS   //  ///////

protected: 		// to permit inheritance

   time_t Ht_t;
   bool local_time;

   static const int days[];

///////
   //   Sets the struct tm depending on local_time status
///////

   void RefreshStructTM() const; 	 // Refresh its content
   struct tm &GetStructTM() const;  // gets it



#ifdef TEST_HTDATETIME

///////
   //   Only for debug: view of struct tm fields
///////

public:

   static void ViewStructTM(struct tm *); 	// view of struct tm fields
   virtual void ViewStructTM();   	  	  	// view of struct tm fields
   void ViewFormats();     	   	       	// View of various formats

   // Test of the class
   static int Test(void);
   static int Test(char **test_dates, const char *format);
   
#endif

};


#endif
