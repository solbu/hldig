#include "CLuceneAPI.h"

#include "CLucene.h"
#include "CLucene/StdHeader.h"

#include "CLucene/util/Reader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/dirent.h"

using namespace std;
using namespace lucene::analysis;
using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::search;
using namespace lucene::document;

static int64_t str = lucene::util::Misc::currentTimeMillis();
static IndexWriter* writer = NULL;
static IndexReader* reader = NULL;
static lucene::analysis::standard::StandardAnalyzer& an = *new lucene::analysis::standard::StandardAnalyzer();

//
// Index close / open
// 
int CLuceneOpenIndex(char * target, int clearIndex)
{
    if ( !clearIndex && IndexReader::indexExists(target) )
    {
        if ( IndexReader::isLocked(target) )
        {
            printf("CLucene index was locked... unlocking it.\n");
            IndexReader::unlock(target);
        }

        writer = _CLNEW IndexWriter( target, &an, false);
    }
    else
    {
        writer = _CLNEW IndexWriter( target ,&an, true);
    }
    writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

    //
    // Create the reader - this will be used for deleting
    // 
    reader = IndexReader::open( target );

    str = lucene::util::Misc::currentTimeMillis();

    return(1);
}

int CLuceneCloseIndex()
{
  	writer->optimize();
    writer->close();
    _CLDELETE(writer);

    delete reader;
    delete writer;
    delete &an;

    cout << _T("Indexing took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

    return(1);
}



//
// Adding
// 
int CLuceneAddDocToIndex(CL_Doc * doc)
{
    //
    // make a new, empty document
    //
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
    // During insert, the field types need to be typecast to non-const,
    // since that is required by CLucene.
    // 

    CL_Doc::iterator i;
    for (i = doc->begin(); i != doc->end(); i++)
    {
        if ((i->second).second == "Keyword")
        {
//            index_doc.add(Field::Keyword((wchar_t*)(i->first).data(), ((i->second).first).data()));
            index_doc.add(*Field::Keyword(
                        utf8_to_wchar((i->first).c_str()),
                        ((i->second).first).c_str()) );
        }
        else if ((i->second).second == "UnIndexed" || (i->second).second == "unIndexed")
        {
//            index_doc.add(*Field::UnIndexed((_CLNEW StringReader(i->first).data()), ((i->second).first).data()));
            index_doc.add(*Field::UnIndexed(
                        utf8_to_wchar((i->first).c_str()),
                        ((i->second).first).c_str()) );

        }
        else if ((i->second).second == "UnStored" || (i->second).second == "unStored") 
        {
//            index_doc.add(*Field::UnStored((_CLNEW StringReader(i->first).data()), ((i->second).first).data()));
            index_doc.add(*Field::UnStored(
                        utf8_to_wchar((i->first).c_str()),
                        ((i->second).first).c_str()) );

        }
        else if ((i->second).second == "Text") 
        {
//            index_doc.add(*Field::Text(_CLNEW StringReader((i->first).data()), ((i->second).first).data()));
            index_doc.add(*Field::Text(
                        utf8_to_wchar((i->first).c_str()),
                        ((i->second).first).c_str()) );

        }
        else
        {
            // Must be some other kind of field (or a typo)...
            // probably safer to ignore it. We could eventually
            // add some kind of logic to decide field type based
            // on common field names, but that might be dangerous
        }
    }

    writer->addDocument( &index_doc );

    delete &index_doc;

    return(1);
}


//
// this function is designed to convery SMALL char*
// from utf8 to wide characters. be careful about
// using it to convert long strings, since this
// function is O(2n) on the number of characters 
//
wchar_t * utf8_to_wchar(const char* value)
{
    const char * counter;
    int len = 0;
        
    //
    // Find the number of characters in the stream.
    //
    counter = value;

    while (counter[0]) {
        if ((counter[0] & 0x80) == 0x00)
        {
            len++;
        }
        else if ((counter[1] & 0xC0) == 0x80)
        {
            if ((counter[0] & 0xE0) == 0xC0)
            {
                len++;
                counter += 2;
            }
            else if (((counter[2] & 0xC0) == 0x80) && ((counter[0] & 0xF0) == 0xE0))
            {
                len++;
                counter += 3;
            }
            else
            {
                counter++; // invalid character
            }
        }
        else
        {
            counter++; // invalid character
        }
    }

    //
    // create a wchar copy of the string
    //
    wchar_t *ucs2_str = (wchar_t *)calloc(len+1, sizeof(wchar_t));
    wchar_t *ucs2_ptr = ucs2_str;
    counter = value;

    while (counter[0])
    {
        if ((counter[0] & 0x80) == 0x00)
        {
            *ucs2_ptr++ = *counter++;
        }
        else if ((counter[1] & 0xC0) == 0x80) 
        {
            if ((counter[0] & 0xE0) == 0xC0)
            {
                *ucs2_ptr++ = ((counter[0] & 0x1F) << 6) | (counter[1] & 0x3F);
                counter += 2;
            }
            else if (((counter[2] & 0xC0) == 0x80) && ((counter[0] & 0xF0) == 0xE0))
            {
                *ucs2_ptr++ = ((counter[0] & 0x0F) << 12) |
                    ((counter[1] & 0x3F) << 6) |
                    (counter[2] & 0x3F);
                counter += 3;
            }
            else
            {
                counter++; // invalid character
            }
        }
        else
        {
            counter++; // invalid character
        }
    }
    *ucs2_ptr = '\0';

    return ucs2_str;
}

//
// Deleting
// 
int CLuceneDeleteURLFromIndex(std::string * url)
{
//    return reader->deleteTerm(new Term( (const wchar_t *) "url", url->c_str() ) );
}


//
// Searching
//
/*

int CLuceneOpenSearch(char * index, char * query)
{
    return(lucene::CLuceneAPI::OpenSearch(query));
}

int CLuceneDoQuery(char * query)
{
    return(lucene::CLuceneAPI::DoQuery(query));
}

int CLuceneSearchGetNth(int n, clucene_query_hit_struct * hit_struct)
{
    Document nth_doc;

    lucene::CLuceneAPI::SearchGetNth(n, nth_doc); 

    //populate hit struct

    return(1);
}

int CLuceneCloseSearch(void)
{
    return(lucene::CLuceneAPI::CloseSearch());
}

*/


