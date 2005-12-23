#ifndef _lucene_test_h_
#define _lucene_test_h_

#include <time.h>
#include <map>
#include <string>

#ifndef LIBHTDIG_API_H

//#define CL_Doc std::map<std::string, std::pair<std::string, std::string> >
//typedef map< string, pair< string, string > > CL_Doc2;
//typedef map<basic_string<char>, pair<basic_string<char>,basic_string<char> > > CL_Doc;

//typedef map<basic_string<char>, pair<basic_string<char_t>, basic_string<char> > > CL_Doc;
//#define clucene_query_hit map<basic_string<char_t>, basic_string<char_t>>

#endif

typedef struct _clucene_query_hit_struct {

//    char_t title[CLUCENE_DOCUMENT_TITLE_L];
//    char_t URL[CLUCENE_MAX_FILENAME_PATH_L];
//    char_t excerpt[CLUCENE_DOCUMENT_EXCERPT_L];
    int  score;
    int  score_percent;     //top result is 100%
    struct tm time_tm;
    int  size;

} clucene_query_hit_struct;

//Indexing

int CLuceneOpenIndex(char * target, int clearIndex);

int CLuceneCloseIndex(void);

int CLuceneAddDocToIndex(std::map<std::string, std::pair<std::string, std::string> > * doc);


//Deleting

int CLuceneDeleteURLFromIndex(std::string * url);


#endif

