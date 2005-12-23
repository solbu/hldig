#include "CLucene/StdHeader.h"

#include "CLucene_API.h"

//#define TR_LEAKS

#ifdef TR_LEAKS 
#ifdef COMPILER_MSVC
#ifdef _DEBUG
	int _lucene_BlockStop;
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif
#endif

#include "SearchFiles.cpp"
#include "IndexFiles.cpp"
#include "DeleteFiles.cpp"
#include "Statistics.cpp"
#include <iostream>

//Indexing

int CLuceneOpenIndex(char * target, int clearIndex)
{
    return(lucene::demo::OpenIndex(target, clearIndex));
}

int CLuceneCloseIndex()
{
    return(lucene::demo::CloseIndex());
}

int CLuceneAddDocToIndex(std::map<std::string, std::pair<std::string, std::string> > * doc)
{
    return(lucene::demo::AddDocToIndex(doc));
}



//Deleting

int CLuceneDeleteURLFromIndex(std::string * url)
{
    return(lucene::demo::DeleteURLFromIndex(url));
}



//Searching

int CLuceneOpenSearch(char * index, char * query)
{
    return(lucene::demo::OpenSearch(query));
}

int CLuceneDoQuery(char * query)
{
    return(lucene::demo::DoQuery(query));
}

int CLuceneSearchGetNth(int n, clucene_query_hit_struct * hit_struct)
{
    Document nth_doc;

    lucene::demo::SearchGetNth(n, nth_doc); 

    //populate hit struct

    return(1);
}

int CLuceneCloseSearch(void)
{
    return(lucene::demo::CloseSearch());
}

