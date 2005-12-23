//#include "stdafx.h"
#ifndef _lucene_demo_IndexFiles_
#define _lucene_demo_IndexFiles_

#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>


using namespace std;

static long_t str = lucene::util::Misc::currentTimeMillis();
static IndexWriter* writer = NULL;
static IndexReader* reader = NULL;
static Directory* d = NULL;
static lucene::analysis::standard::StandardAnalyzer& an = *new lucene::analysis::standard::StandardAnalyzer();

using namespace std;
namespace lucene{ namespace demo {
	using namespace lucene::index;
	using namespace lucene::analysis;
	using namespace lucene::util;
	using namespace lucene::store;
	using namespace lucene::document;

int OpenIndex(char_t* target, int clearIndex)
{
    if ( !clearIndex && IndexReader::indexExists(target) ){
        d = &FSDirectory::getDirectory( target,false );
        if ( IndexReader::isLocked(*d) ){
            _cout << _T("Index was locked... unlocking it.")<<endl;
            IndexReader::unlock(*d);
        }

        writer = new IndexWriter( *d, an, false);
        reader = &IndexReader::open( *d );
    }else{
        d = &FSDirectory::getDirectory(target,true);
        writer = new IndexWriter( *d ,an, true);
        reader = &IndexReader::open( *d );
    }

    return(1);


}

int CloseIndex()
{
    writer->optimize();
    writer->close();
//    reader->close();   ???
    delete reader;
    delete writer;
    delete &an;

    _cout << _T("Indexing took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

    return(1);
}

int AddDocToIndex(std::map<std::string, std::pair<std::string, std::string> > * input_doc)
{
    // make a new, empty document
    Document& index_doc = *new Document();

    // 
    // Add each element of the hash. This assumes that the second
    // part of the value (value is a pair<>) is the CLucene field
    // type. Since there are only four types of input fields:
    //        Keyword  UnIndexed  UnStored  Text
    // we can just use an if statement
    //
    // For every element of the hash, determine which type of field
    // it is by looking at the second element of the pair<> (which
    // is itself the second element of the CL_Doc. Then call index_doc.add
    // with the appropriate field type. The field name is stored
    // in the first string of the CL_Doc, and the field value is 
    // stored in the first value of the pair (which is the second
    // element of the CL_Doc).
    //
    // During insert, the field types need to be typecast to non-
    // const char_t*, since that is required by CLucene.
    // 
    std::map<std::string, std::pair<std::string, std::string> >::iterator i;
    for (i = input_doc->begin(); i != input_doc->end(); i++)
    {
        if ((i->second).second == "Keyword")
        {
            index_doc.add(Field::Keyword((char *)(i->first).data(), ((i->second).first).data()));
        }
        else if ((i->second).second == "UnIndexed" || (i->second).second == "unIndexed")
        {
            index_doc.add(Field::UnIndexed((char *)(i->first).data(), ((i->second).first).data()));
        }
        else if ((i->second).second == "UnStored" || (i->second).second == "unStored") 
        {
            index_doc.add(Field::UnStored((char *)(i->first).data(), ((i->second).first).data()));
        }
        else if ((i->second).second == "Text") 
        {
            index_doc.add(Field::Text((char *)(i->first).data(), ((i->second).first).data()));
        }
        else
        {
            // Must be some other kind of field (or a typo)...
            // probably safer to ignore it. We could eventually
            // add some kind of logic to decide field type based
            // on common field names, but that might be dangerous
        }
    }

    writer->addDocument( index_doc );

    delete &index_doc;

    return(1);


    //
    // Old stuff.... remove when we're sure we won't need it
    // 
    // Add the path of the file as a field named "path".  Use a Text field, so
    // that the index stores the path, and so that the path is searchable
    //doc.add( Field::Text(_T("path"),  simple_doc->location) );

    // Add the last modified date of the file a field named "modified".  Use a
    // Keyword field, so that it's searchable, but so that no attempt is made
    // to tokenize the field into words.
    //doc.add(Field.Keyword("modified",
    //		  DateField.timeToString(f.lastModified())));

    // Add the contents of the file a field named "contents".  Use a Text
    // field, specifying a Reader, so that the text of the file is tokenized.
    //doc.add( Field::Text(_T("title"), simple_doc->title) );
    //doc.add( Field::Text(_T("meta"), simple_doc->meta) ); //UnStored??
    //doc.add( Field::Text(_T("contents"), simple_doc->contents) );

    //char time[25];
    //snprintf(time, 25, "%d", simple_doc->doc_time);
    //time[24] = 0;
    //index_doc.add( Field::UnIndexed(("time"), time) );
}


int DeleteURLFromIndex(std::string * url)
{
    return reader->Delete(new Term("url", url->data() ) );
}




}}


		
#endif

