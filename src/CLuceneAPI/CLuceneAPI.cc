#include "CLuceneAPI.h"

#include "CLucene.h"
#include "CLucene-contrib.h"
    
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
//static IndexReader* reader = NULL;
//static IndexModifier * modifier = NULL;
static PerFieldAnalyzerWrapper* an = NULL;
static char indexDir[256];

#include "HtDebug.h"


//
// The "almost" standard analyzer. it provides stopword
// removal, whitespace, and lowercasing, but doesn't do
// any of the standardAnalyzer tokenizing. essentially
// for when you don't trust StandardAnalyzer
//
CL_NS_DEF2(analysis,almostStandard)

class AlmostStandardAnalyzer : public Analyzer
{
    private:
        CL_NS(util)::CLSetList<TCHAR*> stopSet;
    public:
        AlmostStandardAnalyzer();
        AlmostStandardAnalyzer( TCHAR** stopWords);
        ~AlmostStandardAnalyzer();


        TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};


AlmostStandardAnalyzer::AlmostStandardAnalyzer(): stopSet(false)
{
    StopFilter::fillStopTable( &stopSet,CL_NS(analysis)::StopAnalyzer::ENGLISH_STOP_WORDS);
}

AlmostStandardAnalyzer::AlmostStandardAnalyzer( TCHAR** stopWords): stopSet(false)
{
    StopFilter::fillStopTable( &stopSet,stopWords );
}

AlmostStandardAnalyzer::~AlmostStandardAnalyzer()
{}


TokenStream* AlmostStandardAnalyzer::tokenStream(const TCHAR* fieldName, Reader* reader)
{
    TokenStream* ret = _CLNEW LowerCaseTokenizer(reader);
    ret = _CLNEW StopFilter(ret,true, &stopSet);
    return ret;
}

CL_NS_END2
CL_NS_USE2(analysis,almostStandard)

//
// Index close / open
// 
void CLuceneOpenIndex(char * target, int clearIndex, set<string> * stopWords, bool useStandardAnalyzer)
{
    try
    {
        HtDebug * debug = HtDebug::Instance();
        HtConfiguration * config = HtConfiguration::config();
        wchar_t * stemmer_name;

        strcpy(indexDir, target);

        //
        // figure out the stemmer name
        //
        String cfg_locale = config->Find("locale");
        stemmer_name = utf8_to_wchar(get_stemmer_name(cfg_locale.get()));

        if ( (stemmer_name) && (wcslen(stemmer_name)) )
            debug->outlog(2, "CLuceneAPI: Found [%s] in list of stemmers. get_stemmer_name(..) returns [%s] \n", cfg_locale.get(), get_stemmer_name(cfg_locale.get()));
        else
            debug->outlog(2, "CLuceneAPI: Couldn't find [%s] in list of stemmers.  get_stemmer_name(..) returns [%s]\n", cfg_locale.get(), get_stemmer_name(cfg_locale.get()));

        //
        // Create the analyser first, so writer can have it. the stop words
        // will need to be extracted into a wchar_t array first.
        //
        if (stopWords->size())
        {
            wchar_t ** stopArray = convertStopWords(stopWords);

            if (useStandardAnalyzer)
                an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer(stopArray));
            else
                an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW AlmostStandardAnalyzer(stopArray));

            if ( (stemmer_name) && (wcslen(stemmer_name)))
                an->addAnalyzer(_T("stemmed"), _CLNEW SnowballAnalyzer(stemmer_name, stopArray));
            else
                config->Add("use_stemming", "false"); // no stemming for this run

            for (int i = 0; i<stopWords->size(); i++)
            {
                free(stopArray[i]);
            }
            free(stopArray);
        }
        else
        {
            if (useStandardAnalyzer)
                an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer());
            else
                an = _CLNEW PerFieldAnalyzerWrapper(_CLNEW AlmostStandardAnalyzer());

            if ( (stemmer_name) && (wcslen(stemmer_name)) )
                an->addAnalyzer(_T("stemmed"), _CLNEW SnowballAnalyzer(stemmer_name));
            else
                config->Add("use_stemming", "false"); // no stemming for this run
        }
        free(stemmer_name);

        //
        // add special analyzers for special fields
        //
        an->addAnalyzer(_T("url"), _CLNEW WhitespaceAnalyzer());
        an->addAnalyzer(_T("id"), _CLNEW WhitespaceAnalyzer());
        an->addAnalyzer(_T("author"), _CLNEW WhitespaceAnalyzer());

        debug->outlog(2, "CLuceneAPI: Analysers... ");


        //
        // Create the IndexWriter, unlocking the directory if
        // necessary, and wiping the old index if requested
        //
        Directory* dir = NULL;
        if (IndexReader::indexExists(indexDir))
        {
            if (IndexReader::isLocked(indexDir) )
            {
                debug->outlog(2, "Unlocking index...");
                IndexReader::unlock(indexDir);
            }
            //
            // the index exists, but if clearIndex is set to true, destroy it
            //
            //dir = FSDirectory::getDirectory(indexDir, clearIndex ? true : false);
            writer = _CLNEW IndexWriter( indexDir, an, clearIndex? true : false);
            //writer = _CLNEW IndexWriter( indexDir, an, false);
        }
        else
        {
            //
            // no index exists, so create a new one
            //
            //dir = FSDirectory::getDirectory(indexDir, true);
            writer = _CLNEW IndexWriter( indexDir, an, true);
        }
        //modifier = _CLNEW IndexModifier( indexDir, an, true );
        //
        // set up the max merge docs so the individual index files don't get too large
        // also, using the compound file should be off, since this will bloat file sizes
        //
        //modifier->setMaxMergeDocs(config->Value("clucene_max_merge_docs", 300000));
        //modifier->setUseCompoundFile(false);
        //debug->outlog(2, "IndexModifier... ");


        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
        writer->setMaxMergeDocs(config->Value("clucene_max_merge_docs", 300000));
        debug->outlog(2, "IndexWriter... ");

        //
        // Create the reader - this will be used for deleting
        // 
        //debug->outlog(2, "IndexReader... ");

        //
        // get the start time... useful for debugging
        // 
        str = lucene::util::Misc::currentTimeMillis();

        debug->outlog(2, "created.\n");
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "Exception in CLuceneAPI (CLuceneOpenIndex): [%s]\n", e.what());
        debug->close();
        throw;
    }
}

void CLuceneCloseIndex()
{
    try
    {
        HtDebug * debug = HtDebug::Instance();
        HtConfiguration * config = HtConfiguration::config();

        //
        // commit any deletions
        //
        //reader->commit();

        if (config->Boolean("clucene_optimize"))
        {
            //modifier->optimize();
            writer->optimize();
        }
        //modifier->close();
        writer->close();

        //_CLDELETE(modifier);
        _CLDELETE(writer);
        //_CLDELETE(reader);
        _CLDELETE(an);

        debug->outlog(1, "CLuceneAPI: Indexing took: %dms.\n", lucene::util::Misc::currentTimeMillis() - str);
    }
    catch (CLuceneError & e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "\nException in CLuceneAPI (CLuceneCloseIndex): [%s]\n", e.what());
        debug->close();
        throw;
    }
}



//
// Adding
// 
int CLuceneAddDocToIndex(CL_Doc * doc)
{
    try
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
        //modifier->addDocument( &index_doc );
        writer->addDocument( &index_doc );


        return(1);
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "\nException in CLuceneAPI (CLuceneAddDocToIndex): [%s]\n", e.what());
        debug->close();
        throw;
    }
}


//
// Delete by using the URL
// 
int CLuceneDeleteURLFromIndex(std::string * url)
{
    try
    {
        HtDebug * debug = HtDebug::Instance();
        wchar_t * wtemp = utf8_to_wchar(url->c_str());
        
        writer->close();
        _CLDELETE(writer); 

        Term * tempTerm = _CLNEW Term( _T("url"), wtemp);

        //int result = modifier->deleteDocuments(tempTerm);

        IndexReader * reader = IndexReader::open( indexDir );

        int result = reader->deleteDocuments(tempTerm);

        reader->commit();
        reader->close();

        _CLDELETE(reader);
        _CLDELETE(tempTerm);
        free(wtemp);

        debug->outlog(0, "CLuceneAPI: deleting %s (url field) - deleted %d documents\n", url->c_str(), result);

        writer = _CLNEW IndexWriter(indexDir, an, false);

        return result;
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "\nException in CLuceneAPI (CLuceneDeleteURLFromIndex): [%s]\n", e.what());
        debug->close();
        throw;
    }
}


//
// delete using the id field
//
int CLuceneDeleteIDFromIndex(int id)
{
    try
    {
        HtDebug * debug = HtDebug::Instance();
        wchar_t * wtemp = (wchar_t *)malloc(sizeof(wchar_t) * 32);
        swprintf(wtemp, 31, _T("%d"), id);

        writer->close();
        _CLDELETE(writer); 

        Term * tempTerm = _CLNEW Term( _T("id"), wtemp);

        //int result = modifier->deleteDocuments(tempTerm);

        IndexReader * reader = IndexReader::open( indexDir );

        int result = reader->deleteDocuments(tempTerm);

        reader->commit();
        reader->close();

        _CLDELETE(reader);
        _CLDELETE(tempTerm);
        free(wtemp);

        debug->outlog(0, "CLuceneAPI: deleting %d (id field) - deleted %d documents\n", id, result);

        writer = _CLNEW IndexWriter(indexDir, an, false);

        return result;
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "\nException in CLuceneAPI (CLuceneDeleteIDFromIndex): [%s]\n", e.what());
        debug->close();
        throw;
    }
}


typedef struct stem_data
{
    char stemmer_name[20];
    char name_win32[6];
    char name_solaris[6];
    char name_linux[6];
    char sqlserver[40];
    char oracle[40];
} stem_data;


static stem_data stemmer_langs[] = {
    // stemmer      WIN32  SOLARIS  LINUX   SQL Server Lang        Oracle (NLS_LANG)
    {"english",    "enu", "en_US", "en_US", "English",             "AMERICAN_AMERICA.UTF8"},
    {"english",    "eng", "en_UK", "en_AU", "British English",     "ENGLISH_AUSTRALIA.UTF8"},
    {"english",    "eng", "en_UK", "en_GB", "British English",     "ENGLISH_UNITED KINGDOM.UTF8"},

    {"german",     "deu", "de",    "de_DE", "German",              "GERMAN_GERMANY.UTF8"},

    {"spanish",    "esp", "es",    "es_ES", "Spanish",             "SPANISH_SPAIN.UTF8"},

    {"finnish",    "fin", "en_US", "fi_FI", "Finnish",             "FINNISH_FINLAND.UTF8"},

    {"french",     "frc", "fr_CA", "fr_CA", "French",              "CANADIAN FRENCH_CANADA.UTF8"},
    {"french",     "fra", "fr",    "fr_FR", "French",              "FRENCH_FRANCE.UTF8"},

    {"italian",    "ita", "it",    "it_IT", "Italian",             "ITALIAN_ITALY.UTF8"},

    {"dutch",      "nld", "nl",    "nl_NL", "Dutch",               "DUTCH_THE NETHERLANDS.UTF8"},

    {"portuguese", "ptb", "pt_BR", "pt_BR", "Brazilian",           "BRAZILIAN PORTUGUESE_BRAZIL.UTF8"},

    {"swedish",    "sve", "sv",    "sv_SE", "Swedish",             "SWEDISH_SWEDEN.UTF8"},

    {"danish",     "dan", "da_DK", "da_DK", "Danish",              "DANISH_DENMARK.UTF8"},

    {"norwegian",  "nor", "no_NO", "no_NO", "Norwegian",           "NORWEGIAN_NORWAY.UTF8"},

    {"",           "plk", "pl",    "pl_PL", "Polish",              "POLISH_POLAND.UTF8"},        // no stemmer
    {"",           "csy", "cz",    "cs_CZ", "Czech",               "CZECH_CZECH REPUBLIC.UTF8"}, // no stemmer
    {"",           ""   , "",      "",      "",                    "",},
};


const char * get_stemmer_name(char * input)
{
    int i = 0;
    HtDebug * debug = HtDebug::Instance();
   
    if(!strcmp(input, "C"))
        return NULL;
 
    debug->outlog(2, "CLuceneAPI: get_stemmer_name (%s)\n", input);

    while (strlen(stemmer_langs[i].stemmer_name) > 0)
    {
        if (!strcmp(input, stemmer_langs[i].name_win32) ||
            !strcmp(input, stemmer_langs[i].name_solaris) ||
            !strcmp(input, stemmer_langs[i].name_linux) ||
            !strcmp(input, stemmer_langs[i].sqlserver) ||
            !strcmp(input, stemmer_langs[i].oracle))
        {
                debug->outlog(2, "CLuceneAPI: get_stemmer_name returning [%s]\n", stemmer_langs[i].stemmer_name);
                return stemmer_langs[i].stemmer_name;
        }
        i++;
    }

    debug->outlog(2, "CLuceneAPI: get_stemmer_name returning NULL\n");
    return NULL;
}


//
// return the number of docs in the index. return -1 if the index isn't open
//
int CLuceneNumIndexDocs()
{
    try
    {
        if (writer == NULL)
        {
            return -1;
        }
        else
        {
            return writer->docCount();
        }
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(-1, "\nException in CLuceneAPI (CLuceneNumIndexDocs): [%s]\n", e.what());
        debug->close();
        throw;
    }
}


typedef struct stem_data
{
    char stemmer_name[20];
    char name_win32[6];
    char name_solaris[6];
    char name_linux[6];
    char sqlserver[40];
    char oracle[40];
} stem_data;


static stem_data stemmer_langs[] = {
    // stemmer      WIN32  SOLARIS  LINUX   SQL Server Lang        Oracle (NLS_LANG)
    {"english",    "enu", "en_US", "en_US", "English",             "AMERICAN_AMERICA.UTF8"},
    {"english",    "eng", "en_UK", "en_AU", "British English",     "ENGLISH_AUSTRALIA.UTF8"},
    {"english",    "eng", "en_UK", "en_GB", "British English",     "ENGLISH_UNITED KINGDOM.UTF8"},

    {"german",     "deu", "de",    "de_DE", "German",              "GERMAN_GERMANY.UTF8"},

    {"spanish",    "esp", "es",    "es_ES", "Spanish",             "SPANISH_SPAIN.UTF8"},

    {"finnish",    "fin", "en_US", "fi_FI", "Finnish",             "FINNISH_FINLAND.UTF8"},

    {"french",     "frc", "fr_CA", "fr_CA", "French",              "CANADIAN FRENCH_CANADA.UTF8"},
    {"french",     "fra", "fr",    "fr_FR", "French",              "FRENCH_FRANCE.UTF8"},

    {"italian",    "ita", "it",    "it_IT", "Italian",             "ITALIAN_ITALY.UTF8"},

    {"dutch",      "nld", "nl",    "nl_NL", "Dutch",               "DUTCH_THE NETHERLANDS.UTF8"},

    {"portuguese", "ptb", "pt_BR", "pt_BR", "Brazilian",           "BRAZILIAN PORTUGUESE_BRAZIL.UTF8"},

    {"swedish",    "sve", "sv",    "sv_SE", "Swedish",             "SWEDISH_SWEDEN.UTF8"},

    {"danish",     "dan", "da_DK", "da_DK", "Danish",              "DANISH_DENMARK.UTF8"},

    {"norwegian",  "nor", "no_NO", "no_NO", "Norwegian",           "NORWEGIAN_NORWAY.UTF8"},

    {"",           "plk", "pl",    "pl_PL", "Polish",              "POLISH_POLAND.UTF8"},        // no stemmer
    {"",           "csy", "cz",    "cs_CZ", "Czech",               "CZECH_CZECH REPUBLIC.UTF8"}, // no stemmer
    {"",           ""   , "",      "",      "",                    "",},
};


const char * get_stemmer_name(char * input)
{
    int i = 0;
    HtDebug * debug = HtDebug::Instance();
   
    if(!strcmp(input, "C"))
        return NULL;
 
    debug->outlog(2, "CLuceneAPI: get_stemmer_name (%s)\n", input);

    while (strlen(stemmer_langs[i].stemmer_name) > 0)
    {
        if (!strcmp(input, stemmer_langs[i].name_win32) ||
            !strcmp(input, stemmer_langs[i].name_solaris) ||
            !strcmp(input, stemmer_langs[i].name_linux) ||
            !strcmp(input, stemmer_langs[i].sqlserver) ||
            !strcmp(input, stemmer_langs[i].oracle))
        {
                debug->outlog(2, "CLuceneAPI: get_stemmer_name returning [%s]\n", stemmer_langs[i].stemmer_name);
                return stemmer_langs[i].stemmer_name;
        }
        i++;
    }

    debug->outlog(2, "CLuceneAPI: get_stemmer_name returning NULL\n");
    return NULL;
}


