//
// htnotify.cc
//
// htnotify: Check through databases and look for notify META information
//           Send e-mail to addresses mentioned in documents if the doc
//           has "expired"
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htnotify.cc,v 1.1.2.4 2000/10/20 03:40:59 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtConfiguration.h"
#include "Dictionary.h"
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

// Declare a record for storing email/URL data for later retrieval
class EmailNotification : public Object
{
public:
    EmailNotification (char* date, char* email, char* url, char* subject);

    //
    //accessors
    //
    String getDate()    const { return date; }
    String getEmail()   const { return email; }
    String getUrl()     const { return url; }
    String getSubject() const { return subject; }

private:
    String date;
    String email;
    String url;
    String subject;
};

EmailNotification::EmailNotification (char* pDate, char* pEmail,
                                      char* pUrl,  char* pSubject)
{
    date    = pDate;
    email   = pEmail;
    url     = pUrl;
    if (!pSubject || !*pSubject)
    {
        subject = "page expired";
    }
    else
    {
        subject = pSubject;
    }
}

void htnotify(DocumentRef &);
void usage();
void readPreAndPostamble(void);
void add_notification(char *date, char *email, char *url, char *subject);
void send_notification(char *email, List * notifList);
void send_email(List * notifList, String& command, String& to,
		String& listText, int singleSubject);
int parse_date(char *date, int &year, int &month, int &day);


int	verbose = 0;
int	sendEmail = 1;

//
// This variable is used to hold today's date.  It is global for
// efficiency reasons since computing it is a relatively expensive
// operation
//
struct tm	*today;

//
// This structure holds the set of email notifications requiring
// sending. It is indexed by email address of recipients, and
// each entry is a List of EmailNotification objects.
//
Dictionary  * allNotifications;

//
// These strings holds the preamble/postamble text used in
// email messages.
//
String preambleText;
String postambleText;

//{{{  main
//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
    int			c;
    extern char		*optarg;
    String		base;
    String		configFile = DEFAULT_CONFIG_FILE;

    while ((c = getopt(ac, av, "nvb:c:")) != -1)
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
	case 'n':
	    verbose++;
	    sendEmail = 0;
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

    docdb.Read(doc_db);
    List	*docs = docdb.DocIDs();

    //
    // Compute today's date
    //
    time_t	now = time(0);
    today = localtime(&now);

    readPreAndPostamble();

    //
    // Traverse all the known documents to check for notification requirements
    //
    allNotifications = new Dictionary();
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

    //
    // Iterate through the list of notifications
    //
    allNotifications->Start_Get();
    char * email;
    while ((email = (char *) allNotifications->Get_Next()))
    {
        List * notifList = (List *) allNotifications->Find (email);
        send_notification(email, notifList);
    }

    //
    // tidy up
    //
    docdb.Close();
    delete allNotifications;
    return 0;
}


//}}}  
//{{{  readPreAndPostamble
//*****************************************************************************
// void readPreAndPostamble(void)
//
void readPreAndPostamble(void)
{
    const char* prefixfile = config["htnotify_prefix_file"];
    const char* suffixfile = config["htnotify_suffix_file"];

    // define default preamble text - blank string
    preambleText = "";

    if (prefixfile != NULL)
    {
        ifstream    in(prefixfile);
        char        buffer[1024];

        if (! in.bad())
        {
            while (! in.bad() && ! in.eof())
            {
                in.getline(buffer, sizeof(buffer));
                if (in.eof() && !*buffer)
                    break;
                preambleText << buffer << '\n';
            }
            in.close();
        }
    }

    // define default postamble text
    postambleText = "";
    postambleText << "Note: This message will be sent again if you do not change or\n";
    postambleText << "take away the notification of the above mentioned HTML page.\n";
    postambleText << "\n";
    postambleText << "Find out more about the notification service at\n\n";
    postambleText << "    http://www.htdig.org/meta.html\n\n";
    postambleText << "Cheers!\n\nht://Dig Notification Service\n";

    if (suffixfile != NULL)
    {
        ifstream    in(suffixfile);
        char        buffer[1024];

        if (! in.bad())
        {
            postambleText = "";
            while (! in.bad() && ! in.eof())
            {
                in.getline(buffer, sizeof(buffer));
                if (in.eof() && !*buffer)
                    break;
                postambleText << buffer << '\n';
            }
            in.close();
        }
    }

    if (verbose > 1)
    {
        cout << "Preamble text:" << endl;
        cout << preambleText << endl << endl;
        cout << "Postamble text:" << endl;
        cout << postambleText << endl;
        cout << endl;
    }
}

//}}}  
//{{{  htnotify
//*****************************************************************************
// void htnotify(DocumentRef &ref)
//
void htnotify(DocumentRef &ref)
{
    char	*date = ref.DocNotification();
    char	*email = ref.DocEmail();

    if (date && *date && email && *email)
    {
	if (verbose > 2)
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
	    if (verbose > 2)
	    {
		cout << "Malformed date: " << date << endl;
	    }

	    add_notification(date, email, ref.DocURL(), "Malformed Date");
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
	    add_notification(date, email, ref.DocURL(), ref.DocSubject());
	}
	else
	{
	    // Page not yet expired
	    if (verbose > 2)
	    {
		cout << "htnotify: URL " << ref.DocURL()
		     << " (" << year+1900 << "-" << month+1
		     << "-" << day << ")" << endl;
	    }
	}
    }
}


//}}}  
//{{{  add_notification
//*****************************************************************************
// void add_notification(char *date, char *email, char *url, char *subject)
//
void add_notification(char *date, char *email, char *url, char *subject)
{

    List * list = (List *) allNotifications->Find (email);
    if (list == NULL)
    {   // here's a new recipient so add it
        list = new List();
        allNotifications->Add (email, list);
    }

    // now add the notification to the selected list
    EmailNotification* notif = new EmailNotification(date, email, url, subject);
    list->Add (notif);
}

//}}}  
//{{{  send_notification
//*****************************************************************************
// void send_notification(char * email, List * notifList)
//
void send_notification(char* email, List * notifList)
{
    String	command = SENDMAIL;
    command << " -t";

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
    
    EmailNotification* notif = (EmailNotification*) notifList->Get_First();
    String firstSubject = notif->getSubject();
    int singleSubject = 1;

    //
    // scan to determine whether the same subject message is used throughout
    //
    notifList->Start_Get();
    notifList->Get_Next();
    // continue with the second item in the list
    while ((notif = (EmailNotification*) notifList->Get_Next()))
    {
        String current = notif->getSubject();
        if ( firstSubject != current )
        {
            singleSubject = 0;
            break;
        }
    }


    //
    // Aggregate the list text
    //
    String listText = "";
    notifList->Start_Get();
    while ((notif = (EmailNotification*) notifList->Get_Next()))
    {
        listText << notif->getUrl() << '\n';
        listText << "  expired " << notif->getDate() << "\n";
        if (! singleSubject)
        { listText << "  " << notif->getSubject() << '\n'; }
    }

    if (sendEmail)
    {
        send_email (notifList, command, to, listText, singleSubject);
    }
    else if (verbose)
    {   // just list the notifiable pages
        cout << endl;
        cout << "Notification required to " << to << endl;
        cout << listText;
    }
}


//}}}  
//{{{  send_email
//*****************************************************************************
// void send_email(List * notifList, String& command, String& to)
//
void send_email (List * notifList, String& command,
                 String& to, String& listText, int singleSubject)
{
    String from = "\"";
    from << config["htnotify_webmaster"] << "\" <"
         << config["htnotify_sender"] << ">";

    String replyto = config["htnotify_replyto"];

    if (verbose)
    {
        if (verbose > 1) { cout << endl; }

        cout << "From: " << from << endl;
        cout << "To: " << to << endl;

        if (verbose > 1) { cout << listText; }
    }

    FILE *fileptr;
    if ( (fileptr = popen(command.get(), "w")) != NULL )
    {
        EmailNotification* notif = (EmailNotification*) notifList->Get_First();
        String	out;
        out << "From: " << from << '\n';
        out << "To: " << to << '\n';
        if (replyto.length() > 0)
        { out << "Reply-To: " << replyto << '\n'; }

        if (singleSubject)
        {
            out << "Subject: " << notif->getSubject() << '\n';
        }
        else
        {
            out << "Subject: Web page expiry (" << notif->getSubject() << ", inter alia)\n";
        }

        out << '\n'; // this is the important header/body separator
        out << preambleText;
        out << listText;
        out << postambleText;
        out << '\n';
        fputs( out.get(), fileptr );
        pclose( fileptr );
    }
    else
    {
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
    cout << "\t\tIncrease the verbose level. Use two or three times for\n";
    cout << "\t\tmore output.\n";
    cout << "\t-n\n";
    cout << "\t\tDon't send any email, just list what has expired.\n";
    exit(0);
}


//}}}  
//{{{  parse_date
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
    if (verbose > 2)
	cout << "Date used (y-m-d): " << yy << '-' << mm << '-' << dd << endl;

    year = yy;
    month = mm;
    day = dd;

    return 1;
}


//}}}  
