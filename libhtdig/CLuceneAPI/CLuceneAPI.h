#ifndef _CLuceneAPI_h_
#define _CLuceneAPI_h_

#include <iostream>
#include <time.h>
#include <map>
#include <string>

//#ifdef UNICODE
//  #define CHAR_T wchar_t
//#else
//  #define CHAR_T char
//#endif /* UNICODE */

typedef std::map<
            std::string,
            std::pair<
                std::wstring,
                std::string> >
        CL_Doc;

//Indexing

int CLuceneOpenIndex(char * target, int clearIndex);

int CLuceneCloseIndex(void);

int CLuceneAddDocToIndex(CL_Doc * doc);


//Deleting

int CLuceneDeleteURLFromIndex(std::string * url);


//Utility

wchar_t * utf8_to_wchar(const char* input);


/*
//Searching


typedef struct _clucene_query_hit_struct {

//    char_t title[CLUCENE_DOCUMENT_TITLE_L];
//    char_t URL[CLUCENE_MAX_FILENAME_PATH_L];
//    char_t excerpt[CLUCENE_DOCUMENT_EXCERPT_L];
    int  score;
    int  score_percent;     //top result is 100%
    struct tm time_tm;
    int  size;

} clucene_query_hit_struct;

int CLuceneOpenSearch(char * target);

int CLuceneDoQuery(char * query_text);

int CLuceneSearchGetNth(int, clucene_query_hit_struct *);

int CLuceneCloseSearch(void);
*/

#endif // _CLuceneAPI_h_

