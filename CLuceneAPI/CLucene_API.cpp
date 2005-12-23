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

#include "IndexFiles.cpp"
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




