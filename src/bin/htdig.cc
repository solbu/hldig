//
// htdig.cc
// 
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htdig.cc,v 1.1.2.1 2006/09/26 00:14:15 aarnone Exp $
//


// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
  #include <getopt.h>
#elif HAVE_GETOPT_LOCAL
  #include <getopt_local.h>
#endif

#include <unistd.h>


#include <iostream>
using namespace std;

#include "libhtdig_api.h"
#include "Spider.h"

void usage();

//
// Start of the program.
//
int main(int ac, char **av)
{
    int             c;
    extern char		*optarg;

    String          minimalFile = 0;
    htdig_parameters_struct * params = (htdig_parameters_struct *)malloc(sizeof(htdig_parameters_struct)); 


    //
    // set up the parameter defaults
    //
    params->configFile[0] = '\0';
    params->DBpath[0] = '\0';
    params->credentials[0] = '\0';
    params->minimalFile[0] = '\0';

    //debugging & logfile
    params->logFile[0] = '\0';   //location of log file
    params->debugFile[0] = '\0';   //location of debug file
    params->debug = 0;        //0, 1 ,2, 3, 4, 5

    //boolean values
    params->initial = 0;
    params->create_text_database = 0;
    params->report_statistics = 0;
    params->alt_work_area = 0;
    params->use_cookies = 1;

    //spidering filters
    params->URL[0] = '\0';
    params->limit_urls_to[0] = '\0';
    params->limit_normalized[0] = '\0';
    params->exclude_urls[0] = '\0';
    params->search_restrict[0] = '\0';
    params->search_exclude[0] = '\0';
    params->search_alwaysreturn[0] = '\0';
    params->url_rewrite_rules[0] = '\0';
    params->bad_querystr[0] = '\0';

    //misc overrides
    params->locale[0] = '\0';
    params->max_hop_count[0] = '\0';     //9 digit limit
    params->max_head_length[0] = '\0';   //9 digit limit
    params->max_doc_size[0] = '\0';      //9 digit limit


    //
    // Parse command line arguments, overriding parameter defaults
    //
    while ((c = getopt(ac, av, "lsm:c:vith:u:a")) != -1)
    {
        unsigned int pos;
        switch (c)
        {
            case 'c':
                strcpy (params->configFile, optarg);
                break;
            case 'v':
                params->debug++;
                break;
            case 'i':
                params->initial = 1;
                break;
            case 't':
                params->create_text_database = 1;
                break;
            case 'h':
                strcpy(params->max_hop_count, optarg);
                break;
            case 's':
                params->report_statistics = 1;
                break;
            case 'u':
                strcpy(params->credentials, optarg);
                for (pos = 0; pos < strlen(optarg); pos++)
                    optarg[pos] = '*';
                break;
            case 'a':
                params->alt_work_area = 1;
                break;
            case 'm':
                minimalFile = optarg;
                break;
            case '?':
            default:
                usage();
                break;
        }
    }

    //1) htdig_open(.....)
    //2) htdig_spider(.....)
    //3) htdig_close()
    

    //
    // if a minimal file was specified
    //
    if (minimalFile.length() != 0)
    {
        //
        // this modification to the max_hop_count is stated
        // in the ht://Dig help file
        //
        strcpy (params->max_hop_count, "0");

        //
        // Handle list of URLs given in a file (stdin, if "-") specified as
        // argument to -m or as an optional trailing argument.
        // 
        if (optind < ac)
        {
            if (params->debug)
            {
                if (minimalFile.length() != 0)
                {
                    cout << "Warning: argument " << av[optind] 
                        << " overrides -m " << minimalFile << endl;
                }
            }
            minimalFile = av[optind];
        }
        if (strcmp (minimalFile.get(), "-") == 0)
        {
            String str;
            // Why not combine this with the code below, with  input = stdin ?
            while (!cin.eof())
            {
                cin >> str;
                str.chop("\r\n");       // (Why "\r\n" here and "\r\n\t " below?)
                if (str.length() > 0)
                {
                    strcpy(params->URL, str.get());
                }
            }
        }
        else if (minimalFile.length() != 0)
        {
            FILE *input = fopen(minimalFile.get(), "r");
            char buffer[1000];

            if (input)
            {
                while (fgets(buffer, sizeof(buffer), input))
                {
                    String str(buffer);
                    str.chop("\r\n\t ");
                    if (str.length() > 0)
                    {
                        strcpy(params->URL, str.get());
                    }
                }
                fclose(input);
            }
            else
            {
                cerr << "Could not open argument '" << minimalFile
                    << "' of flag -m\n";
                exit (1);
            }
        }
    }

    Spider * spider;
    //
    // traditional
    //
/*
    spider = new Spider(params);
    spider->openDBs(params);
    spider->Start(params);
    spider->closeDBs();
    delete spider;
*/




//
// DEBUGGING CRAP
//


/* 
    cout << "_____________" << endl;

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "opening DBs" << endl;
    spider->openDBs(params);

    cout << "deleting by id" << endl;
    spider->DeleteDoc(55);

    cout << "deleting by URL" << endl;
    string * tempURL = new string("http://tethys/");
    spider->DeleteDoc(tempURL);

    cout << "closing DBs" << endl;
    spider->closeDBs();

    cout << "deleting spider" << endl;
    delete spider;
*/

    cout << "_____________" << endl;

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "deleting spider" << endl;
    delete spider;

/* 
    cout << "_____________" << endl;

    cout << "setting debug file" << endl;
    strcpy(params->debugFile, "temp.txt");

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "opening DBs" << endl;
    spider->openDBs(params);

    singleDoc newDoc;

    cout << "going to add single doc by URL" << endl;
    newDoc["url"] = "http://tethys/";
    newDoc["id"] = "55";
    spider->addSingleDoc(&newDoc, 0, 1, false);

    cout << "deleting by id" << endl;
    spider->DeleteDoc(55);

    cout << "deleting by URL" << endl;
    string * tempURL = new string("http://tethys/");
    spider->DeleteDoc(tempURL);

    cout << "closing DBs" << endl;
    spider->closeDBs();

    cout << "deleting spider" << endl;
    delete spider;
*/
/*
    cout << "_____________" << endl;

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "going to retrieve a URL using the spider" << endl;
    string blah = "http://cs.montana.edu/~arnone/";
    singleDoc * newDoc2 = spider->fetchSingleDoc(&blah);

    cout << "--RETRIEVED" << endl;
    cout << "--Title:"<< (*newDoc2)["title"] << endl;
    cout << "--Body:"<< (*newDoc2)["contents"] << endl;

    cout << "deleting spider" << endl;
    delete spider;



    cout << "_____________" << endl;

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "deleting spider" << endl;
    delete spider;



    cout << "_____________" << endl;

    cout << "creating spider" << endl;
    spider = new Spider(params);

    cout << "going to retrieve a URL using the spider" << endl;
    string blah2 = "http://cs.montana.edu/~arnone/";
    singleDoc * newDoc3 = spider->fetchSingleDoc(&blah2);

    cout << "--RETRIEVED" << endl;
    cout << "--Title:"<< (*newDoc3)["title"] << endl;
    cout << "--Body:"<< (*newDoc3)["contents"] << endl;

    cout << "deleting spider" << endl;
    delete spider;
*/
}


//
// Display usage information for the htdig program
//
void usage()
{
    cout << "usage: htdig [-v][-i][-c configfile][-t][-m minimalfile]\n";
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

    cout << "\t-m minimalfile  (or just a file name at end of arguments)\n";
    cout << "\t\tTells htdig to read URLs from the supplied file and index\n";
    cout << "\t\tthem in place of (or in addition to) the existing URLs in\n";
    cout << "\t\tthe database and the start_url.  With the -m, only the\n";
    cout << "\t\tURLs specified are added to the database.  A file name of\n";
    cout << "\t\t'-' indicates the standard input.\n\n";


	
    exit(0);
}

