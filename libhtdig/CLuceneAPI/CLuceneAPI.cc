#include "CLuceneAPI.h"

#include "CLucene.h"
    
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(document)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,standard)
#ifndef _MSC_VER
CL_NS_USE2(analysis,snowball)
#endif

static int64_t str = lucene::util::Misc::currentTimeMillis();
static IndexWriter* writer = NULL;
static IndexReader* reader = NULL;
static PerFieldAnalyzerWrapper* an = NULL;
#ifndef _MSC_VER
static SnowballAnalyzer* san = NULL;
#endif

extern int debug;

//
// Index close / open
// 
void CLuceneOpenIndex(char * target, int clearIndex, set<string> * stopWords)
{
    HtConfiguration * config = HtConfiguration::config();

    //
    // Create the analyser first, so writer can have it. the stop words
    // will need to be extracted into a wchar_t array first.
    //
    if (stopWords->size())
    {
        wchar_t ** stopArray = convertStopWords(stopWords);

        an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer(stopArray));

#ifndef _MSC_VER
        san = _CLNEW SnowballAnalyzer(_T("english"), stopArray);
        an->addAnalyzer(_T("stemmed"), san);
#endif

        for (int i = 0; i<stopWords->size(); i++)
        {
            free(stopArray[i]);
        }
        free(stopArray);
    }
    else
    {
        an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer());

#ifndef _MSC_VER
        san = _CLNEW SnowballAnalyzer(_T("english"));
        an->addAnalyzer(_T("stemmed"), san);
#endif
    }
    if (debug > 2)
        cout << "Analysers... ";

    //
    // Create the IndexWriter, unlocking the directory if
    // necessary, and wiping the old index is requested
    //
    if ( !clearIndex && IndexReader::indexExists(target) )
    {
        if ( IndexReader::isLocked(target) )
        {
            if (debug > 2)
                cout << "Unlocking CLucene...";
            IndexReader::unlock(target);
        }

        writer = _CLNEW IndexWriter( target, an, false);
    }
    else
    {
        writer = _CLNEW IndexWriter( target, an, true);
    }
    if (debug > 2)
        cout << "IndexWriter... ";
    writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    writer->setMaxMergeDocs(config->Value("clucene_max_merge_docs", 300000));

    //
    // Create the reader - this will be used for deleting
    // 
    reader = IndexReader::open( target );
    if (debug > 2)
        cout << "IndexReader... ";

    //
    // get the start time... useful for debugging
    // 
    str = lucene::util::Misc::currentTimeMillis();

    if (debug > 2)
        cout << "created." << endl;
}

void CLuceneCloseIndex()
{
    HtConfiguration * config = HtConfiguration::config();

    if (config->Boolean("clucene_optimize"))
    {
        writer->optimize();
    }
    writer->close();

    //
    // commit any deletions
    //
    reader->commit();

    _CLDELETE(writer);
    _CLDELETE(reader);
    _CLDELETE(an);
    //_CLDELETE(san); // deleteing an will do this ???

    if (debug > 1)
        cout << _T("Indexing took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;
}



//
// Adding
// 
int CLuceneAddDocToIndex(CL_Doc * doc)
{
    //
    // make a new, empty document (make sure it's a CLucene document)
    //
    lucene::document::Document index_doc;

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
    // During insert, the field types need to be typecast to non-const,
    // since that is required by CLucene.
    // 

    CL_Doc::iterator i;
    for (i = doc->begin(); i != doc->end(); i++)
    {
        if ((i->second).second == "Keyword" || (i->second).second == "keyword")
        {
            wchar_t * temp = utf8_to_wchar((i->first).c_str());
            index_doc.add(*Field::Keyword(temp, ((i->second).first).c_str()) );
            free(temp);
        }
        else if ((i->second).second == "UnIndexed" || (i->second).second == "unIndexed")
        {
            wchar_t * temp = utf8_to_wchar((i->first).c_str());
            index_doc.add(*Field::UnIndexed(temp, ((i->second).first).c_str()) );
            free(temp);
        }
        else if ((i->second).second == "UnStored" || (i->second).second == "unStored") 
        {
            wchar_t * temp = utf8_to_wchar((i->first).c_str());
            index_doc.add(*Field::UnStored(temp, ((i->second).first).c_str()) );
            free(temp);
        }
        else if ((i->second).second == "Text" || (i->second).second == "text")
        {
            wchar_t * temp = utf8_to_wchar((i->first).c_str());
            index_doc.add(*Field::Text(temp, ((i->second).first).c_str()) );
            free(temp);
        }
        else
        {
            //
            // Must be some other kind of field (or a typo)...
            // probably safer to ignore it. We could eventually
            // add some kind of logic to decide field type based
            // on common field names, but that sounds dangerous
            //
        }
    }

    //
    // document assembled; add to index
    //
    writer->addDocument( &index_doc );


    return(1);
}


//
// Delete by using the URL
// 
int CLuceneDeleteURLFromIndex(std::string * url)
{
    wchar_t * wtemp;
    Term * tempTerm;
    int result;

    wtemp = utf8_to_wchar(url->c_str());
    tempTerm = new Term( _T("url"), wtemp);

    wcout << "deleting " << tempTerm->field() << ":" << tempTerm->text() << endl;

    result = reader->deleteDocuments(tempTerm);

    delete tempTerm;
    free(wtemp);

    if (debug)
    {
        cout << "CLucene: deleting " << *url << " - deleted " << result << " documents" << endl;
    }
    return result;
}


//
// delete using the doc-id field
//
int CLuceneDeleteIDFromIndex(int id)
{
    char temp[32];
    wchar_t * wtemp;
    Term * tempTerm;
    int result;

    sprintf(temp, "%d", id);
    wtemp = utf8_to_wchar(temp);
    tempTerm = new Term( _T("doc-id"), wtemp);

    result = reader->deleteDocuments(tempTerm);

    delete tempTerm;
    free(wtemp);
    free(temp);

    return result;
}



