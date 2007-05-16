#ifndef _CLuceneAPI_h_
#define _CLuceneAPI_h_

#include "HtStdHeader.h"
#include "HtConfiguration.h"


// 
// Index operations
// 

void CLuceneOpenIndex(char * target, int clearIndex, set<string> * stopWords, bool useStandardAnalyzer);

void CLuceneCloseIndex(void);

int CLuceneAddDocToIndex(CL_Doc * doc);


// 
// Deleting
// 

int CLuceneDeleteURLFromIndex(string * url);

int CLuceneDeleteIDFromIndex(int id);

//
// utility
//
const char * get_stemmer_name(char * input);

//
// Utility
//

int CLuceneNumIndexDocs(void);

const char * get_stemmer_name(char * input);

#endif // _CLuceneAPI_h_

