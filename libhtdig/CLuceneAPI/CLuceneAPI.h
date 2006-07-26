#ifndef _CLuceneAPI_h_
#define _CLuceneAPI_h_

#include "HtStdHeader.h"
#include "HtConfiguration.h"


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

int CLuceneDeleteIDFromIndex(int id);


#endif // _CLuceneAPI_h_

