//
// htnotify.cc
//
// htnotify: Check through databases and look for notify META information
//           Send e-mail to addresses mentioned in documents if the doc
//           has "expired"
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htnotify.cc,v 1.24 2000/02/19 05:29:05 ghutchis Exp $
//

#include "HtConfiguration.h"
#include "DocumentDB.h"
#include "DocumentRef.h"
#include "defaults.h"
#include "HtURLCodec.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

void htnotify(DocumentRef &);
void usage();
void send_notification(char *date, char *email, char *url, char *subject);
int parse_date(char *date, int &year, int &month, int &day);


int	verbose = 0;

//
// This variable is used to hold today's date.  It is global for
// efficiency reasons since computing it is a relatively expensive
// operation
//
struct tm	*today;


//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
    int			c;
    extern char		*optarg;
    String		base;
    String		configFile = DEFAULT_CONFIG_FILE;

    while ((c = getopt(ac, av, "vb:c:")) != -1)
    {
	switch (c)
	{
	case 'b':
	    base = optarg;
	    break;
	case 'c':
	    configFile = optarg;
	    break;
	case 'v':
	    verbose++;
	    break;
	case '?':
	    usage();
	    break;
	}
    }

    config.Defaults(&defaults[0]);
    config.Read(configFile);

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance()->ErrMsg();

    if (url_part_errors.length() != 0)
    {
      cerr << form("htnotify: Invalid url_part_aliases or common_url_parts: %s",
                   url_part_errors.get()) << endl;
      exit (1);
    }

    if (base.length())
    {
	config.Add("database_base", base);
    }

    String	doc_db = config["doc_db"];
    DocumentDB	docdb;

    // Check "uncompressed"/"uncoded" urls at the price of time
    // (extra DB probes).
    docdb.SetCompatibility(config.Boolean("uncoded_db_compatible", 1));

    docdb.Read(doc_db);
    List	*docs = docdb.DocIDs();

    //
    // Compute today's date
    //
    time_t	now = time(0);
    today = localtime(&now);

    //
    // Traverse all the known documents to check for notification requirements
    //
    DocumentRef	*ref;
    IntObject		*id;
    docs->Start_Get();
    while ((id = (IntObject *) docs->Get_Next()))
    {
	ref = docdb[id->Value()];
	if (ref)
	    htnotify(*ref);
	delete ref;
    }
    delete docs;
    docdb.Close();
    return 0;
}


//*****************************************************************************
// void htnotify(DocumentRef &ref)
//
void htnotify(DocumentRef &ref)
{
    char	*date = ref.DocNotification();
    char	*email = ref.DocEmail();

    if (date && *date && email && *email)
    {
	if (verbose > 1)
	{
	    cout << "Saw a date:" << endl;
	    cout << "Date:    " << date << endl;
	    cout << "URL:     " << ref.DocURL() << endl;
	    cout << "Subject: " << ref.DocSubject() << endl;
	    cout << "Email:   " << email << endl;
	    cout << endl;
	}

	int		month, day, year;
	if (!parse_date(date, year, month, day))
	{
	    // Parsing Failed
	    if (verbose > 1)
	    {
		cout << "Malformed date: " << date << endl;
	    }

	    send_notification(date, email, ref.DocURL(), "Malformed Date");

	    if (verbose)
	    {
		cout << "Message sent." << endl;
		cout << "Date:    " << date << endl;
		cout << "URL:     " << ref.DocURL() << endl;
		cout << "Subject: Malformed Date" << endl;
		cout << "Email:   " << email << endl;
		cout << endl;
	    }
	    return;
	}

	year -= 1900;
	month--;

	//
	// Compare this date with today's date
	//
	if (year < today->tm_year ||
	    (year == today->tm_year && month < today->tm_mon) ||
	    (year == today->tm_year && month == today->tm_mon &&
	     day < today->tm_mday))
	{
	    //
	    // It seems that this date is either today or before
	    // today.  Send a notification
	    //
	    send_notification(date, email, ref.DocURL(), ref.DocSubject());
	    if (verbose)
	    {
		cout << "Message sent." << endl;
		cout << "Date:    " << date << endl;
		cout << "URL:     " << ref.DocURL() << endl;
		cout << "Subject: " << ref.DocSubject() << endl;
		cout << "Email:   " << email << endl;
		cout << endl;
	    }
	}
	else
	{
	    // Page not yet expired
	    if (verbose)
	    {
		cout << "htnotify: URL " << ref.DocURL()
		     << " (" << year+1900 << "-" << month+1
		     << "-" << day << ")" << endl;
	    }
	}
    }
}


//*****************************************************************************
// void send_notification(char *date, char *email, char *url, char *subject)
//
void send_notification(char *date, char *email, char *url, char *subject)
{
    String	command = SENDMAIL;
    command << " -t -F '\"ht://Dig Notification Service\"' -f \"";
    command << config["htnotify_sender"] << '"';

    String	em = email;
    String	to = "";
    char	*token = strtok(em.get(), " ,\t\r\n");
    while (token)
    {
	if (*token)
	{
	    if (to.length())
		to << ", ";
	    to << token;
	}
	token = strtok(0, " ,\t\r\n");
    }

// Before we use the email address string, we may want to sanitize it.
//    static char ok_chars[] = "abcdefghijklmnopqrstuvwxyz
//    ABCDEFGHIJKLMNOPQRSTUVWXYZ
//    1234567890_-.@/=+:%!, ";
//    char *cursor;          // cursor into email address 
//    for (cursor = to.get(); *(cursor += strspn(cursor, ok_chars));)
//      *cursor = '_'; // Set it to something harmless
    
    FILE *fileptr;
    if ( (fileptr = popen(command.get(), "w")) != NULL ) {

      if (!subject || !*subject)
	subject = "page expired";
      String	out;
      out << "From: \"ht://Dig Notification Service\" <"
	  << config["htnotify_sender"] << ">\n";
      out << "Subject: WWW notification: " << subject << '\n';
      out << "To: " << to.get() << '\n';
      out << "Reply-To: " << config["htnotify_sender"] << "\n";
      out << "\n";
      out << "The following page was tagged to notify you after " << date
	  << '\n';
      out << "\n";
      out << "URL:     " << url << '\n';
      out << "Date:    " << date << '\n';
      out << "Subject: " << subject << '\n';
      out << "Email:   " << email << '\n';
      out << "\n";
      out << "Note: This message will be sent again if you do not change or\n";
      out << "take away the notification of the above mentioned HTML page.\n";
      out << "\n";
      out << "Find out more about the notification service at\n\n";
      out << "    http://www.htdig.org/meta.html\n\n";
      out << "Cheers!\n\nht://Dig Notification Service\n";

      fputs( out.get(), fileptr );
      pclose( fileptr );
    } else {
      perror( "popen" );
    }

}


//*****************************************************************************
// Display usage information for the htnotify program
//
void usage()
{
    cout << "usage: htnotify [-c configfile][-b db_base]\n";
    cout << "This program is part of ht://Dig " << VERSION << "\n\n";
    cout << "There can be any number or words.\n";
    cout << "Options:\n";
    cout << "\t-c configfile\n";
    cout << "\t\tUse the specified configuration file instead of the default.\n\n";
    cout << "\t-b db_base\n";
    cout << "\t\tSet the base path of the document database.\n";
    cout << "\t-v\n";
    cout << "\t\tIncrease the verbose level by one.\n";
    exit(0);
}


//*****************************************************************************
// Parse the notification date string from the user's document
//
int parse_date(char *date, int &year, int &month, int &day)
{
    int		mm = -1, dd = -1, yy = -1, t;
    String	scandate = date;

    for (char *s = scandate.get(); *s; s++)
	if (ispunct(*s))
	    *s = ' ';

    if (config.Boolean("iso_8601"))
    {
	// conf file specified ISO standard, so expect [yy]yy mm dd.
	sscanf(scandate.get(), "%d%d%d", &yy, &mm, &dd);
    }
    else
    {
	// Default to American standard when not specified in conf,
	// so expect mm dd [yy]yy.
	sscanf(scandate.get(), "%d%d%d", &mm, &dd, &yy);
	if (mm > 31 && dd <= 12 && yy <= 31)
	{
	    // probably got yyyy-mm-dd instead of mm/dd/yy
	    t = mm; mm = dd; dd = yy; yy = t;
	}
    }

    // OK, we took our best guess at the order the y, m & d should be.
    // Now let's see if we guessed wrong, and fix it.  This won't work
    // for ambiguous dates (e.g. 01/02/03), which must be given in the
    // expected format.
    if (dd > 31 && yy <= 31)
    {
	t = yy; yy = dd; dd = t;
    }
    if (mm > 31 && yy <= 31)
    {
	t = yy; yy = mm; mm = t;
    }
    if (mm > 12 && dd <= 12)
    {
	t = dd; dd = mm; mm = t;
    }
    if (yy < 0 || mm < 1 || mm > 12 || dd < 1 || dd > 31)
	return 0;		// Invalid date

    if (yy < 70)		// before UNIX Epoch
	yy += 2000;
    else if (yy < 1900)		// before computer age
	yy += 1900;
    if (verbose > 1)
	cout << "Date used (y-m-d): " << yy << '-' << mm << '-' << dd << endl;

    year = yy;
    month = mm;
    day = dd;

    return 1;
}
