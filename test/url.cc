//
// url.cc
//
// url: Implement tests for the URL parser
//	Should ensure compliance to RFC2396
//	<http://www.faqs.org/rfcs/rfc2396.html>
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: url.cc,v 1.9 2004/06/20 12:02:41 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "HtConfiguration.h"
#include "URL.h"


// These should probably be tested individually
// but for now, we'll just assume they're set to defaults
// (except for external protocol test).
static ConfigDefaults defaults[] = {
  { "external_protocols", "https:// dummy.transport help: dummy.transport", 0 },
  { "allow_virtual_hosts", "true", 0 },
  { "case_sensitive", "true", 0 },
  { "remove_default_doc", "index.html", 0 },
  { "server_aliases", "alias.com:443=true.com:443", 0 },
  { 0 }
};

typedef struct {
  char* url_parents;
  char* url_children;
  int test_children;
} params_t;


static void usage();
static void dourl(params_t* params);
static void dolist(params_t* params);

static int verbose = 0;

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.url_parents = strdup("url.parents");
  params.url_children = strdup("url.children");
  params.test_children = 1;

  while ((c = getopt(ac, av, "vop:c:")) != -1)
    {
      switch (c)
	{
	case 'v':
	  verbose++;
	  break;
	case 'p':
	  free(params.url_parents);
	  params.url_parents = strdup(optarg);
	  break;
	case 'c':
	  free(params.url_children);
	  params.url_children = strdup(optarg);
	  break;
	case 'o' :
	  params.test_children = 0;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  dourl(&params);

  free(params.url_parents);
  free(params.url_children);

  return 0;
}

static void dourl(params_t* params)
{
  if(verbose) cerr << "Test WordKey class with " <<
		params->url_parents << " and " << params->url_children << "\n";
  HtConfiguration* const config= HtConfiguration::config();
  config->Defaults(defaults);
  dolist(params);

  cout << "\nAnd now without // -> / , case-insensitive and considering spaces...\n\n";
  config->Add(String("allow_double_slash"), "true");
  config->Add(String("allow_space_in_url"), "true");
  config->Add(String("case_sensitive"), "false");
  // Can't also test "remove_default_doc", since the library only reads that
  // attribute once...
  dolist(params);
}

static void dolist(params_t* params)
{
  // To start, we read in the list of child URLs into a List object
  FILE          *urllist = fopen(params->url_children, "r");
  char          buffer[1000];
  List		children;
 
  if (params->test_children)
    {
      while (fgets(buffer, sizeof(buffer), urllist))
	{
	  buffer [sizeof(buffer) - 1] = '\0';	// make strlen() safe
	  int len = strlen(buffer);
	  if (len && buffer [len-1] == '\n')
	      buffer [len-1] = '\0';		// remove trailing '\n'
	  children.Add(new String(buffer));
	}
      fclose(urllist);
    }

  urllist = fopen(params->url_parents, "r");
  URL parent, child;
  String *current;
  while (fgets(buffer, sizeof(buffer), urllist))
    {
      parent = URL(buffer);
      cout << "Parent: " << buffer << '(' << parent.signature().get() << ")\n";
      parent.dump();
      if (params->test_children)
	{
	  cout << "\nChildren: \n";
	  children.Start_Get();
	  while ((current = (String *)children.Get_Next()))
	    {
	      cout << "\nChild: " << current->get() << endl;
	      child = URL(current->get(), parent);
	      child.dump();
	    }
	  cout << endl;
	}
    }

  fclose(urllist);
  children.Destroy();
}

//*****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    cout << "usage: url [options]\n";
    cout << "Options:\n";
    cout << "\t-v\t\tIncreases the verbosity\n";
    cout << "\t-p file\tname of the url parent file\n";
    cout << "\t-c file\tname of the url children file\n";
    exit(0);
}



