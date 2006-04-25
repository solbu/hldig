#include "CLuceneAPI.h"

#include "CLucene.h"
    
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(search)
CL_NS_USE(document)
CL_NS_USE(queryParser)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,standard)

static int64_t str = lucene::util::Misc::currentTimeMillis();
static IndexWriter* writer = NULL;
static IndexReader* reader = NULL;
static IndexSearcher* searcher = NULL;
static StandardAnalyzer* an = NULL;
static Hits* hits = NULL;
static QueryParser * parser = NULL;

extern int debug;

//
// Index close / open
// 
void CLuceneOpenIndex(char * target, int clearIndex, set<string> * stopWords)
{
    //
    // Create the analyser first, so writer can have it. the stop words
    // will need to be extracted into a wchar_t array first
    //
    if (stopWords->size())
    {
        wchar_t ** stopArray = convertStopWords(stopWords);

        an = _CLNEW StandardAnalyzer(stopArray);

        for (int i = 0; i<stopWords->size(); i++)
        {
            free(stopArray[i]);
        }
        free(stopArray);
    }
    else
    {
        an = _CLNEW StandardAnalyzer();
    }

    //
    // Create the query parser.. this needs to be declared
    // beforehand so we can set options for it
    //
    parser = _CLNEW QueryParser(_T("contents"), an);

    //
    // Create the IndexWriter, unlocking the directory if
    // necessary, and wiping the old index is requested
    //
    if ( !clearIndex && IndexReader::indexExists(target) )
    {
        if ( IndexReader::isLocked(target) )
        {
            if (debug > 1)
                printf("CLucene index was locked... unlocking it.\n");
            IndexReader::unlock(target);
            if (debug > 1)
                printf("Unlock sucessful\n");
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

    //
    // Create the reader - this will be used for deleting
    // 
    reader = IndexReader::open( target );
    if (debug > 2)
        cout << "IndexReader... ";

    //
    // Create the searcher - this will be used for.... searching!
    //
	searcher = _CLNEW IndexSearcher(reader);
    if (debug > 2)
        cout << "IndexSearcher... ";

    //
    // get the start time... useful for debugging
    // 
    str = lucene::util::Misc::currentTimeMillis();

    if (debug > 2)
        cout << "created." << endl;
}

void CLuceneCloseIndex()
{
    writer->optimize();
    writer->close();

    _CLDELETE(writer);
    _CLDELETE(reader);
    _CLDELETE(an);
    _CLDELETE(searcher);
    _CLDELETE(hits);

    //    cout << _T("Indexing took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;
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


wchar_t** convertStopWords(set<string> * stopWords)
{
    wchar_t ** stopArray = (wchar_t **)calloc(stopWords->size(), sizeof(wchar_t *));

    if (!stopArray)
    {
        return NULL;
    }

    set<string>::iterator i;
    int j = 0;
    for (i = stopWords->begin(); i != stopWords->end(); i++) {
        stopArray[j] = utf8_to_wchar(i->c_str());
        j++;
    }

    return stopArray;
}


//
// Deleting
// 
int CLuceneDeleteURLFromIndex(std::string * url)
{
    wchar_t * temp = utf8_to_wchar(url->c_str());
    int result = reader->deleteTerm(new Term( _T("url"), temp));
    free(temp);

    return result;
}


//
// Searching
//

void changeDefaultOperator()
{
    parser->setOperator(QueryParser::AND_OPERATOR);
}

int CLuceneDoQuery(string * input)
{
    //
    // contents is the default field. the query will need
    // to contain references to the other fields. eg: title:(query text)
    //
    wchar_t * temp = utf8_to_wchar(input->c_str());
    //Query * query = parser->parse(temp, _T("contents"), an);
    Query * query = parser->parse(temp);

    if (debug > 2)
    {
        wchar_t * converted_query = query->toString(_T("contents"));
        char * converted_query_utf8 = wchar_to_utf8(converted_query);
        cout << "Converted query before searching: " << converted_query_utf8 << endl;
        free(converted_query);
        free(converted_query_utf8);
    }

    hits = searcher->search( query );
    if (debug > 1)
    {
        cout << "CLucene API: Search sucessful" << endl;
    }
    

    free(temp);
    return hits->length();
}

void CLuceneSearchGetNth(int n, htsearch_query_match_struct * hit_struct)
{
    lucene::document::Document doc = hits->doc(n);
    char * temp;

    temp = wchar_to_utf8(doc.get(_T("url")));
    strcpy (hit_struct->URL, temp);
    free(temp);

    temp = wchar_to_utf8(doc.get(_T("title")));
    strcpy (hit_struct->title, temp);
    free(temp);

    temp = wchar_to_utf8(doc.get(_T("size")));
    hit_struct->size = atoi(temp);
    free(temp);

    temp = wchar_to_utf8(doc.get(_T("time")));
    time_t tempTime = (time_t)atoi(temp);
    gmtime_r(&tempTime, &(hit_struct->time_tm));
    free(temp);

    hit_struct->score = (int)(hits->score(n) * 10000);
    hit_struct->score_percent = hit_struct->score / 100;

// 
// populate hit struct from doc, using wchar_to_utf8
//
// definition:
//
/*
typedef struct htsearch_query_match_struct {

    char title[HTDIG_DOCUMENT_TITLE_L];
    char URL[HTDIG_MAX_FILENAME_PATH_L];
    char excerpt[HTDIG_DOCUMENT_EXCERPT_L];
    int  score;
    int  score_percent;     //top result is 100%
    struct tm time_tm;
    int  size;

} htsearch_query_match_struct;
*/
}




