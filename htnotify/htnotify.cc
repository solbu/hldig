//
// htnotify.cc
//
// Implementation of htnotify
//
// $Log: htnotify.cc,v $
// Revision 1.8  1998/11/02 20:36:30  ghutchis
//
// Changed HTDig to ht://Dig.
//
// Revision 1.7  1998/10/02 17:07:32  ghutchis
//
// More configure changes
//
// Revision 1.6  1998/09/23 14:58:22  ghutchis
//
// Many, many bug fixes
//
// Revision 1.5  1998/08/03 16:50:44  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.4  1998/07/22 10:04:28  ghutchis
//
// Added patches from Sylvain Wallez <s.wallez.alcatel@e-mail.com> to
// Display.cc to use the filename if no title is found and Chris Jason
// Richards <richards@cs.tamu.edu> to htnotify.cc to fix problems with sendmail.
//
// Revision 1.3  1997/06/23 02:27:24  turtle
// Added version info to the usage output
//
// Revision 1.2  1997/03/13 18:37:50  turtle
// Changes
//
// Revision 1.1.1.1  1997/02/03 17:11:11  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: htnotify.cc,v 1.8 1998/11/02 20:36:30 ghutchis Exp $";
#endif

#include <Configuration.h>
#include <DocumentDB.h>
#include <DocumentRef.h>
#include <defaults.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream.h>
#include <time.h>
#include <stdio.h>


void htnotify(DocumentRef &);
void usage();
void send_notification(char *date, char *email, char *url, char *subject);


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
    extern char	*optarg;
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

    if (base.length())
    {
	config.Add("database_base", base);
    }

    String	doc_db = config["doc_db"];
    DocumentDB	docdb;
    docdb.Read(doc_db);
    List	*docs = docdb.URLs();

    //
    // Compute today's date
    //
    time_t	now = time(0);
    today = localtime(&now);

    //
    // Traverse all the known documents to check for notification requirements
    //
    DocumentRef	*ref;
    String		*str;
    docs->Start_Get();
    while ((str = (String *) docs->Get_Next()))
    {
	ref = docdb[str->get()];
	htnotify(*ref);
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
	if (config.Boolean("iso_8601"))
	  {
	    sscanf(date, "%d-%d-%d", &year, &month, &day);
	  }
	else
	  {
	    sscanf(date, "%d/%d/%d", &month, &day, &year);
	  }

	if (year > 1900)
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
    }
}


//*****************************************************************************
// void send_notification(char *date, char *email, char *url, char *subject)
//
void send_notification(char *date, char *email, char *url, char *subject)
{
  /* Currently unused    int		fildes[2]; */
    String	to = email;

    String command = SENDMAIL;
    command << "-F \"ht://Dig Notification Service\" -f ";
    command << config["htnotify_sender"];

    char *token = strtok(to, " ,\t\r\n");
    while (token)
    {
      command << " " << token;
      token = strtok(0, " ,\t\r\n");
    }
    
    FILE *fileptr;
    if( (fileptr = popen(command.get(), "w")) != NULL ) {

      if (!subject || !*subject)
	subject = "notification";
      String	out;
      out << "From: " << config["htnotify_sender"] << "\n";
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
// Display usage information for the htdig program
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


