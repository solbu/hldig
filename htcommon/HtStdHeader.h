#ifndef _HtStdHeader_h_
#define _HtStdHeader_h_

//
// standard C++ headers
//
#include <time.h>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef _MSC_VER // _WIN32 
  #include <pwd.h>
#else
  #include <sys/types.h>
  #include <winsock2.h>
#endif // _WIN32
#include <unistd.h>

using namespace std;


//
// #define statements
//
#define URL_SEPCHARS " ,"

//
// typedefs 
//

typedef map<string, string> singleDoc;
typedef set<string> uniqueWordsSet;
typedef map<string, pair<wstring, string> > CL_Doc;

/*
//
// libhtdig headers - these need to be taken out
//
#include "libhtdig_log.h"
#include "libhtdig_api.h"



//
// ht://Dig specific headers
//
#ifdef HAVE_CONFIG_H
  #include "htconfig.h"
#endif //HAVE_CONFIG_H
#include "HtConfiguration.h"

#include "StringList.h"
#include "Dictionary.h"
#include "Queue.h"
#include "List.h"
#include "StringList.h"
#include "HtRegexList.h"
#include "WordType.h"
#include "md5.h"
#include "good_strtok.h"

#include "URLRef.h"
#include "Server.h"
#include "Document.h"
#include "DocumentRef.h"
#include "IndexDB.h"
#include "IndexDBRef.h"
#include "TidyParser.h"
#include "CLuceneAPI.h"

#include "Transport.h"
#include "HtHTTP.h"			  // For HTTP statistics
#include "defaults.h"

#include "HtDateTime.h"
#include "HtURLRewriter.h"
#include "HtURLCodec.h"

////////////////////////////
// For cookie jar
////////////////////////////
#include "HtCookieJar.h"
#include "HtCookieMemJar.h"
#include "HtCookieInFileJar.h"
#include "HtHTTP.h"
////////////////////////////


//
// variables (should be extern and defined in HtStdHeader.cc)
//
extern String configFile;
extern int debug;


*/



//
// functions
//

wchar_t * utf8_to_wchar(const char* input);

char * wchar_to_utf8(const wchar_t* input);


#endif // _HtStdHeader_h_

