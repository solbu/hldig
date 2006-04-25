#ifndef _CLuceneAPI_h_
#define _CLuceneAPI_h_

#include "HtStdHeader.h"


//
// ugly, but needed for the htsearch_query_match_struct
//
#include "libhtdig_api.h"


// 
// Index operations
// 

void CLuceneOpenIndex(char * target, int clearIndex, set<string> * stopWords);

void CLuceneCloseIndex(void);

int CLuceneAddDocToIndex(CL_Doc * doc);


// 
// Deleting
// 

int CLuceneDeleteURLFromIndex(string * url);


// 
// Searching
// 

void changeDefaultOperator();

int CLuceneDoQuery(string * query_text);

void CLuceneSearchGetNth(int, htsearch_query_match_struct *);



// 
// Utility
// 

wchar_t** convertStopWords(set<string> * stopWords);

#endif // _CLuceneAPI_h_

