//
// main.cc
// 
// Indexes the web sites specified in the config file
// generating several databases to be used by htmerge
//
// $Log: main.cc,v $
// Revision 1.12  1999/01/21 13:41:25  ghutchis
// Check HtURLCodec for errors.
//
// Revision 1.11  1999/01/17 20:32:21  ghutchis
// Added support for url_log, save and restart digs.
//
// Revision 1.10  1999/01/02 16:13:30  bergolth
// added warning message if locale selection failed
//
// Revision 1.9  1998/12/19 18:09:03  bergolth
// Added bad_querystr option.
//
// Revision 1.8  1998/12/19 14:39:42  bergolth
// Added StringList::Join and fixed URL::removeIndex.
//
// Revision 1.7  1998/12/05 00:52:55  ghutchis
//
// Added a parameter to Initial function to prevent URLs from being checked
// twice during an update dig.
//
// Revision 1.6  1998/12/04 04:13:51  ghutchis
// Use configure check to only include getopt.h when it exists.
//
//

#include "Document.h"
#include "Retriever.h"
#include "StringList.h"
#include "htdig.h"
#include <defaults.h>
#include <HtURLCodec.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

//
// Global variables
//
int			debug = 0;
int			report_statistics = 0;
DocumentDB		docs;
StringMatch		limits;
StringMatch		limitsn;
StringMatch		excludes;
StringMatch             badquerystr;
FILE			*urls_seen = NULL;
FILE			*images_seen = NULL;
String			configFile = DEFAULT_CONFIG_FILE;

void usage();
void reportError(char *msg);


//
// Start of the program.
//
main(int ac, char **av)
{
    int			c;
    extern char		*optarg;
    String		credentials;
    int			initial = 0;
    int			alt_work_area = 0;
    int			create_text_database = 0;
    char		*max_hops = 0;
    RetrieverLog	flag  = Retriever_noLog;
	
    //
    // Parse command line arguments
    //
    while ((c = getopt(ac, av, "lsc:vith:u:a")) != -1)
    {
	switch (c)
	{
	    case 'c':
		configFile = optarg;
		break;
	    case 'v':
		debug++;
		break;
	    case 'i':
		initial++;
		break;
	    case 't':
		create_text_database++;
		break;
	    case 'h':
		max_hops = optarg;
		break;
	    case 's':
		report_statistics++;
		break;
	    case 'u':
		credentials = optarg;
		break;
	    case 'a':
		alt_work_area++;
		break;
	    case 'l':
		flag = Retriever_logUrl;
		break;
	    case '?':
		usage();
	}
    }

    //
    // First set all the defaults and then read the specified config
    // file to override the defaults.
    //
    config.Defaults(&defaults[0]);
    if (access(configFile, R_OK) < 0)
    {
	reportError(form("Unable to find configuration file '%s'",
			 configFile.get()));
    }
    config.Read(configFile);

    if (*config["locale"] == '\0' && debug > 0)
      cout << "Warning: unknown locale!\n";

    if (max_hops)
    {
	config.Add("max_hop_count", max_hops);
    }

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance()->ErrMsg();

    if (url_part_errors.length() != 0)
      reportError(form("Invalid url_part_aliases or common_url_parts: %s",
                       url_part_errors.get()));

    //
    // If indicated, change the database file names to have the .work
    // extension
    //
    if (alt_work_area != 0)
    {
	String	configValue;

	configValue = config["doc_db"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("doc_db", configValue);
	}

	configValue = config["word_list"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("word_list", configValue);
	}
    }
    
    //
    // If needed, we will create a list of every URL we come across.
    //
    if (config.Boolean("create_url_list"))
    {
	String	filename = config["url_list"];
	urls_seen = fopen(filename, initial ? "w" : "a");
	if (urls_seen == 0)
	{
	    reportError(form("Unable to create URL file '%s'",
			     filename.get()));
	}
    }

    //
    // If needed, we will create a list of every image we come across.
    //
    if (config.Boolean("create_image_list"))
    {
	String	filename = config["image_list"];
	images_seen = fopen(filename, initial ? "w" : "a");
	if (images_seen == 0)
	{
	    reportError(form("Unable to create images file '%s'",
			     filename.get()));
	}
    }

    //
    // Set up the limits list
    //
    StringList l(config["limit_urls_to"], " \t");
    limits.IgnoreCase();
    limits.Pattern(l.Join('|'));
    l.Release();

    l.Create(config["limit_normalized"], " \t");
    limitsn.IgnoreCase();
    limitsn.Pattern(l.Join('|'));
    l.Release();

    //
    // Patterns to exclude from urls...
    //
    l.Create(config["exclude_urls"], " \t");
    excludes.IgnoreCase();
    excludes.Pattern(l.Join('|'));
    l.Release();

    l.Create(config["bad_querystr"], " \t");
    badquerystr.IgnoreCase();
    badquerystr.Pattern(l.Join('|'));
    l.Release();

    //
    // Open the document database
    //
    String		filename = config["doc_db"];
    if (initial)
	unlink(filename);
    if (docs.Open(filename) < 0)
    {
	reportError(form("Unable to open/create document database '%s'",
			 filename.get()));
    }

    if (initial)
    {
	filename = config["word_list"];
	unlink(filename);
    }

    //
    // Create the Retriever object which we will use to parse all the
    // HTML files.
    // In case this is just an update dig, we will add all existing
    // URLs?
    //
    Retriever	retriever(flag);
    List	*list = docs.URLs();
    retriever.Initial(*list);
    delete list;

    // Add start_url to the initial list of the retriever.
    // Don't check a URL twice!
    // Beware order is important, if this bugs you could change 
    // previous line retriever.Initial(*list, 0) to Initial(*list,1)
    if (credentials.length())
	retriever.setUsernamePassword(credentials);
    retriever.Initial(config["start_url"], 1);

    //
    // Go do it!
    //
    retriever.Start();

    //
    // All done with parsing.
    //

    //
    // If the user so wants, create a text version of the document database.
    //
    if (create_text_database)
    {
	filename = config["doc_list"];
	if (initial)
	    unlink(filename);
	docs.CreateSearchDB(filename);
    }

    //
    // Cleanup
    //
    if (urls_seen)
	fclose(urls_seen);
    if (images_seen)
	fclose(images_seen);

    //
    // If needed, report some statistics
    //
    if (report_statistics)
    {
	retriever.ReportStatistics("htdig");
    }
}


//
// Display usage information for the htdig program
//
void usage()
{
    cout << "usage: htdig [-v][-i][-c configfile][-t]\n";
    cout << "This program is part of ht://Dig " << VERSION << "\n\n";
    cout << "Options:\n";

    cout << "\t-v\tVerbose mode.  This increases the verbosity of the\n";
    cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
    cout << "\t\tfor debugging purposes.  The default verbose mode\n";
    cout << "\t\tgives a nice progress report while digging.\n\n";

    cout << "\t-i\tInitial.  Do not use any old databases.  This is\n";
    cout << "\t\taccomplished by first erasing the databases.\n\n";

    cout << "\t-c configfile\n";
    cout << "\t\tUse the specified configuration file instead of the\n";
    cout << "\t\tdefault.\n\n";

    cout << "\t-t\tCreate an ASCII version of the document database.\n";
    cout << "\t\tThis database is easy to parse with other programs so\n";
    cout << "\t\tthat information can be extracted from it.\n\n";

    cout << "\t-h hopcount\n";
    cout << "\t\tLimit the stored documents to those which are at\n";
    cout << "\t\tmost hopcount links away from the start URL.\n\n";

    cout << "\t-s\tReport statistics after completion.\n\n";

    cout << "\t-u username:password\n";
    cout << "\t\tTells htdig to send the supplied username and\n";
    cout << "\t\tpassword with each HTTP request.  The credentials\n";
    cout << "\t\twill be encoded using the 'Basic' authentication scheme.\n";
    cout << "\t\tThere *HAS* to be a colon (:) between the username\n";
    cout << "\t\tand password.\n\n";

    cout << "\t-a\tUse alternate work files.\n";
    cout << "\t\tTells htdig to append .work to database files, causing\n";
    cout << "\t\ta second copy of the database to be built.  This allows\n";
    cout << "\t\tthe original files to be used by htsearch during the\n";
    cout << "\t\tindexing run.\n\n";
	
    // FIXME
    cout << "\t-l\tStop and restart.\n";

    exit(0);
}

//
// Report an error and die
//
void reportError(char *msg)
{
    cout << "htdig: " << msg << "\n\n";
    exit(1);
}

