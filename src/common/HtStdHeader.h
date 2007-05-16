#ifndef _HtStdHeader_h_
#define _HtStdHeader_h_

//
// standard C++ headers
//
#include <time.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32 
  #include <sys/types.h>
  #include <winsock2.h>
#else
  #include <pwd.h>
  #include <unistd.h>
#endif // _WIN32

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

//
// functions
//

wchar_t * utf8_to_wchar(const char* input);

char * wchar_to_utf8(const wchar_t* input);

void sanitize_utf8_string(char * value);

//
// must free the returned wchar_t array!!!!!
//
wchar_t** convertStopWords(set<string> * stopWords);

#endif // _HtStdHeader_h_

