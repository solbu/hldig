//----------------------------------------------------------------
//
// libhtdig_api.h
//
// Header function for htdig shared library API
//
// 1/25/2002 created
//
// Neal Richter nealr@rightnow.com
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_api.h,v 1.1.2.1 2006/09/25 23:50:49 aarnone Exp $
//
//----------------------------------------------------------------

#ifndef LIBHTDIG_API_H
#define LIBHTDIG_API_H

#include <time.h>

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef DLLEXPORT
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif
#endif

#define HTDIG_MAX_FILENAME_PATH_L            1024
#define HTDIG_DOCUMENT_ID_L                    32
#define HTDIG_DOCUMENT_TITLE_L                256
#define HTDIG_DOCUMENT_META_L                4096
#define HTDIG_DOCUMENT_CONTENT_TYPE_L          32
#define HTDIG_DOCUMENT_EXCERPT_L              1024  
//make sure HTDIG_DOCUMENT_EXCERPT_L is more than config 'excerpt_length'

//default failsafe size of 'excerpt' document
//make sure it's more than config 'max_head_length'
#define HTDIG_DEFAULT_EXCERPT_SIZE         524288

//should be the same as the default value in HTDIG
#define HTDIG_MAX_QUERY_L                     1024       


#define HTDIG_CUSTOM_TEXT_MIME_TYPE           "text/vnd.customdocument"


//searching
#define  HTSEARCH_ALG_AND                   0x00000100         //"and"
#define  HTSEARCH_ALG_AND_STR               "and"

#define  HTSEARCH_ALG_BOOLEAN               0x00000001         //"boolean"
#define  HTSEARCH_ALG_BOOLEAN_STR           "boolean"

#define  HTSEARCH_ALG_OR                    0x00000010         //"or"
#define  HTSEARCH_ALG_OR_STR                "or"


#define  HTSEARCH_FORMAT_LONG               0x00000001         //"long"
#define  HTSEARCH_FORMAT_LONG_STR           "long"

#define  HTSEARCH_FORMAT_SHORT              0x00000010         //"short"
#define  HTSEARCH_FORMAT_SHORT_STR          "short"


#define  HTSEARCH_SORT_SCORE                0x00000001         //"score"
#define  HTSEARCH_SORT_SCORE_STR            "score"

#define  HTSEARCH_SORT_REV_SCORE            0x00000010         //"reverse score"
#define  HTSEARCH_SORT_REV_SCORE_STR        "reverse score"

#define  HTSEARCH_SORT_TIME                 0x00000100         //"time"
#define  HTSEARCH_SORT_TIME_STR             "time"

#define  HTSEARCH_SORT_REV_TIME             0x00001000         //"reverse time"
#define  HTSEARCH_SORT_REV_TIME_STR         "reverse time"

#define  HTSEARCH_SORT_TITLE                0x00010000         //"title"
#define  HTSEARCH_SORT_TITLE_STR            "title"

#define  HTSEARCH_SORT_REV_TITLE            0x00100000         //"reverse title"
#define  HTSEARCH_SORT_REV_TITLE_STR        "reverse title"



#define  HTDIG_ERROR_INDEX_NOT_OPEN            -100
#define  HTDIG_ERROR_CONFIG_READ               -101
#define  HTDIG_ERROR_URL_PART                  -102
#define  HTDIG_ERROR_URL_REWRITE               -103
#define  HTDIG_ERROR_URL_CREATE_FILE           -104
#define  HTDIG_ERROR_IMAGE_CREATE_FILE         -105
#define  HTDIG_ERROR_OPEN_CREATE_DOCDB         -106
#define  HTDIG_ERROR_LOGFILE_OPEN              -107
#define  HTDIG_ERROR_LOGFILE_CLOSE             -108
#define  HTDIG_ERROR_EMPTY_INDEX               -109

#define  HTDIG_ERROR_ADD_SINGLE_RETRIEVE       -120

#define  HTDIG_ERROR_TESTURL_EXCLUDE           -175
#define  HTDIG_ERROR_TESTURL_BADQUERY          -176
#define  HTDIG_ERROR_TESTURL_EXTENSION         -177
#define  HTDIG_ERROR_TESTURL_EXTENSION2        -178
#define  HTDIG_ERROR_TESTURL_LIMITS            -179
#define  HTDIG_ERROR_TESTURL_LIMITSNORM        -180
#define  HTDIG_ERROR_TESTURL_SRCH_RESTRICT     -181
#define  HTDIG_ERROR_TESTURL_SRCH_EXCLUDE      -182
#define  HTDIG_ERROR_TESTURL_REWRITE_EMPTY     -183
#define  HTDIG_ERROR_TESTURL_ROBOT_FORBID      -184

#define  HTSEARCH_ERROR_NO_MATCH               -201
#define  HTSEARCH_ERROR_BAD_MATCH_INDEX        -202
#define  HTSEARCH_ERROR_BAD_DOCUMENT           -203
#define  HTSEARCH_ERROR_TEMPLATE_ERROR         -204
#define  HTSEARCH_ERROR_LOGFILE_OPEN           -205
#define  HTSEARCH_ERROR_LOGFILE_CLOSE          -206
#define  HTSEARCH_ERROR_CONFIG_READ            -207
#define  HTSEARCH_ERROR_URL_PART               -208
#define  HTSEARCH_ERROR_WORDDB_READ            -209
#define  HTSEARCH_ERROR_DOCINDEX_READ          -210
#define  HTSEARCH_ERROR_DOCDB_READ             -211
#define  HTSEARCH_ERROR_EXCERPTDB_READ         -212
#define  HTSEARCH_ERROR_QUERYPARSER_ERROR      -213
#define  HTSEARCH_ERROR_MATCHLIST_ERROR        -214
#define  HTSEARCH_ERROR_INDEX_NOT_FOUND        -215

#define  HTMERGE_ERROR_LOGFILE_OPEN            -301
#define  HTMERGE_ERROR_LOGFILE_CLOSE           -302
#define  HTMERGE_ERROR_CONFIG_READ             -303
#define  HTMERGE_ERROR_URL_PART                -304
#define  HTMERGE_ERROR_WORDDB_READ             -305
#define  HTMERGE_ERROR_DOCINDEX_READ           -306
#define  HTMERGE_ERROR_DOCDB_READ              -307
#define  HTMERGE_ERROR_EXCERPTDB_READ          -308

#define  PHP_HTDIG_CONFIGFILE_PARM              "configFile"
#define  PHP_HTDIG_URL_PARM                     "URL"
#define  PHP_HTDIG_LIMITTO_PARM                 "limit_urls_to"
#define  PHP_HTDIG_LIMITN_PARM                  "limit_normalized"
#define  PHP_HTDIG_EXCLUDEURLS_PARM             "exclude_urls"
#define  PHP_HTDIG_SEARCHRESTRICT_PARM          "search_restrict"
#define  PHP_HTDIG_SEARCHEXCLUDE_PARM           "search_exclude"
#define  PHP_HTDIG_MAXHOPCOUNT_PARM             "max_hop_cont"
#define  PHP_HTDIG_URLREWRITE_PARM              "url_rewrite_rules"
#define  PHP_HTDIG_BAD_QUERYSTR_PARM            "bad_querystr"

//=============================================================================
//===== HTDIG INDEXING API ====================================================


/***************************************************
 * HTDIG_DOCUMENTATION for htdig_parameters_struct
 *
 *    DEBUGGING PARAMETERS
 *
 *    int debug 
 *        Verbose mode.  This increases the verbosity of the
 *         program.  Using more than 2 is probably only useful
 *         for debugging purposes.  The default verbose mode
 *         gives a nice progress report while digging.
 *
 *    char logFile
 *         File to stream debugging & error messages to!
 *    
 *    BOOLEAN PARAMETERS
 *
 *    int initial
 *         Initial.  Do not use any old databases.  This is
 *        accomplished by first erasing the databases
 *
 *    int create_text_database
 *         Create an ASCII version of the document database.
 *        This database is easy to parse with other programs so
 *         that information can be extracted from it.
 *
 *    int report_statistics
 *         Report statistics after completion.
 *
 *    int alt_work_area
 *         Use alternate work files.
 *        Tells htdig to append .work to database files, causing
 *        a second copy of the database to be built.  This allows
 *        the original files to be used by htsearch during the
 *        indexing run.
 * 
 *        
 *    STRING PARAMETERS
 *
 *    char configFile
 *         configfile
 *         Use the specified configuration file instead of the
 *         default.
 *
 *    char credentials
 *        username:password
 *        Tells htdig to send the supplied username and
 *        password with each HTTP request.  The credentials
 *        will be encoded using the 'Basic' authentication scheme.
 *        There *HAS* to be a colon (:) between the username
 *        and password.
 *
 *
 *    char maxhops    //9 digit limit
 *         hopcount
 *         Limit the stored documents to those which are at
 *         most hopcount links away from the start URL.
 *
 *    char minimalFile
 *
 *    char URL
 *         'command-line' URLs from stdin
 *         fetches & indexes these URLs
 *
 ******************************************************************/

typedef struct htdig_parameters_struct {

  char configFile[HTDIG_MAX_FILENAME_PATH_L];
  char DBpath[HTDIG_MAX_FILENAME_PATH_L];
  char credentials[HTDIG_MAX_FILENAME_PATH_L];
  char minimalFile[HTDIG_MAX_FILENAME_PATH_L];

  //debugging & logfile
  char logFile[HTDIG_MAX_FILENAME_PATH_L];   //location of log file
  char debugFile[HTDIG_MAX_FILENAME_PATH_L];   //location of debug messages file
  int debug;            // debug level - 0, 1 ,2, 3, 4, 5
  
  //boolean values
  int initial;
  int create_text_database;
  int report_statistics;
  int alt_work_area;
  int use_cookies;

  //spidering filters
  char URL[HTDIG_MAX_FILENAME_PATH_L];
  char limit_urls_to[HTDIG_MAX_FILENAME_PATH_L];
  char limit_normalized[HTDIG_MAX_FILENAME_PATH_L];
  char exclude_urls[HTDIG_MAX_FILENAME_PATH_L];
  char search_restrict[HTDIG_MAX_FILENAME_PATH_L];
  char search_exclude[HTDIG_MAX_FILENAME_PATH_L];
  char search_alwaysreturn[HTDIG_MAX_FILENAME_PATH_L];
  char url_rewrite_rules[HTDIG_MAX_FILENAME_PATH_L];
  char bad_querystr[HTDIG_MAX_FILENAME_PATH_L];
  
  //misc overrides
  char locale[16];
  char title_factor[16];
  char text_factor[16];
  char meta_description_factor[16];
  char max_hop_count[10];     //9 digit limit
  char max_head_length[10];   //9 digit limit
  char max_doc_size[10];      //9 digit limit
  
  //the rewritten URL - OUTGOING after htdig_index_test_url
  char rewritten_URL[HTDIG_MAX_FILENAME_PATH_L];
  
} htdig_parameters_struct;

/*****************************************************************
 *  HTDIG_DOCUMENTATION for htdig_simple_doc_struct
 *
 *   STRING PARAMETERS
 *   
 *    char location
 *          the 'URL' of the document.  Can be any usefull string.
 *          
 *    char documentid
 *          document id of document  [NOT CURRENTLY USED - IGNORED]
 *          
 *    char title
 *          document title
 *          
 *    char meta
 *          content that is indexed but won appear in an search excerpts
 *          
 *    char * contents
 *          pointer to a NULL TERMINATED string on information to be
 *          indexed.
 *          
 *    char content_type
 *          a MIME-like string
 *          custom MIME-type defined above, others are supported by 
 *          htdig as well.
 *      
 * 
 *****************************************************************/

typedef struct htdig_simple_doc_struct {
    
    char location[HTDIG_MAX_FILENAME_PATH_L];
    char documentid [HTDIG_DOCUMENT_ID_L];
    char title[HTDIG_DOCUMENT_TITLE_L];
    char meta[HTDIG_DOCUMENT_META_L];
    char *contents;                                     //MUST ALLOCATE & FREE!!!
    char content_type[HTDIG_DOCUMENT_CONTENT_TYPE_L];   //MIME-ISH string
    time_t doc_time;
    int spiderable;
    int content_length;
    
} htdig_simple_doc_struct;


DLLEXPORT int htdig_index_open(htdig_parameters_struct *);
DLLEXPORT int htdig_index_urls(htdig_parameters_struct * );
DLLEXPORT int htdig_index_simple_doc(htdig_simple_doc_struct *);
DLLEXPORT htdig_simple_doc_struct * htdig_fetch_simple_doc(char * );
DLLEXPORT int htdig_remove_doc_by_url(char *);
DLLEXPORT int htdig_remove_doc_by_id(int);
DLLEXPORT int htdig_index_close(void);



DLLEXPORT int htdig_get_max_head_length(void);
DLLEXPORT int htdig_index_test_url(htdig_parameters_struct * );




//==============================================================================
//===== HTDIG SEARCHING API ====================================================

/************************************************
 *  HTDIG_DOCUMENTATION for htsearch_parameters_struct
 * 
 *   DEBUGGING PARAMETERS
 *
 *   int debug 
 *       Verbose mode.  This increases the verbosity of the;
 *       program.  Using more than 2 is probably only useful;
 *       for debugging purposes.  The default verbose mode;
 *       gives a progress on what it is doing and where it is.;
 *
 *   char logFile
 *        File to stream debugging & error messages to!
 *
 *   STRING PARAMETERS
 *       
 *   char configFile
 *       configfile
 *       Use the specified configuration file instead of the default.
 *
 *
 **************************************************/

typedef struct htsearch_parameters_struct {

  char configFile[HTDIG_MAX_FILENAME_PATH_L];
  char DBpath[HTDIG_MAX_FILENAME_PATH_L];
//  char aliasesPath[HTDIG_MAX_FILENAME_PATH_L];
//  char stopwordsPath[HTDIG_MAX_FILENAME_PATH_L];
  char locale[16];

  //debugging & logfile
  char logFile[HTDIG_MAX_FILENAME_PATH_L];   //location of log file
  char debugFile[HTDIG_MAX_FILENAME_PATH_L];   //location of log file
  int debug;            //0, 1 ,2, 3, 4, 5
 
  //filters
  char search_restrict[HTDIG_MAX_FILENAME_PATH_L];
  char search_exclude[HTDIG_MAX_FILENAME_PATH_L];
  char search_alwaysreturn[HTDIG_MAX_FILENAME_PATH_L];
  char title_factor[16];
  char text_factor[16];
  char meta_description_factor[16];
  
} htsearch_parameters_struct;




/*****************************************************************
 *  HTDIG_DOCUMENTATION for htsearch_query_struct
 *
 *  STRING PARAMETERS
 *  
 *       char raw_query
 *          STRING of text that is the search query -- syntax is important
 *
 *  INTEGER PARAMETERS      
 *  
 *      int algorithms_flag    [ALSO CALLED 'method' IN HTDIG]
 *          HTSEARCH_ALG_BOOLEAN
 *          HTSEARCH_ALG_OR
 *          HTSEARCH_ALG_AND
 *  
 *      int sortby_flag
 *          score, date, title & reversed
 *          HTSEARCH_SORT_SCORE
 *          HTSEARCH_SORT_REV_SCORE 
 *          HTSEARCH_SORT_TIME
 *          HTSEARCH_SORT_REV_TIME 
 *          HTSEARCH_SORT_TITLE 
 *          HTSEARCH_SORT_REV_TITLE
 *    
 *      int format
 *          short, long (with excerpt)
 *          HTSEARCH_FORMAT_LONG
 *          HTSEARCH_FORMAT_SHORT 
 *
 *
 * 
 *  TODO:  'Connect' these htsearch features to this API
 *
 *  config
 *    Specifies the name of the configuration file.
 *    
 *  exclude
 *    This value is a pattern that specifies which URLs are to be excluded from
 *    the search results.
 *    
 *  keywords
 *    Used to specify a list of required words that have to be in the documents.
 *    
 *  restrict
 *    This value is a pattern that all URLs of the search results will have to 
 *    match.
 *    
 *  startyear, startmonth, startday, endyear, endmonth, endday
 *    These values specify the allowed range of document modification dates 
 *    allowed in the search results. 
 *
 *
 * 
 *****************************************************************/

typedef struct htsearch_query_struct {

  char raw_query[HTDIG_MAX_QUERY_L];
  
  int  algorithms_flag;
  int  sortby_flag;
  int format;
  
} htsearch_query_struct;


/*****************************************************************
 *  HTDIG_DOCUMENTATION for htsearch_query_match_struct
 *
 *  STRING PARAMETERS
 * 
 *     char title
 *          Title of document returned
 *          
 *     char URL
 *          URL/location-string of document returned
 *          
 *     char excerpt
 *          Excerpt with search words highlighted with 
 *          <strong>searchword</strong>
 *
 *  INTEGER PARAMETERS
 *   
 *     int  score   
 *          score in 'number of stars'  
 *          [MAX NUMBER OF STARS DECLARED IN CONFIG FILE]
 *          
 *     int  score_percent     //top result is 100%
 *
 *     time_t time  [DOCUMENT TIME]
 *     struct tm time_tm    [DOCUMENT TIME]
 *     int  size  [TOTAL DOCUMENT SIZE]
 *
 * 
 *****************************************************************/

typedef struct htsearch_query_match_struct {

    char title[HTDIG_DOCUMENT_TITLE_L];
    char URL[HTDIG_MAX_FILENAME_PATH_L];
    char excerpt[HTDIG_DOCUMENT_EXCERPT_L];
    int  id;
    int  score;
    int  score_percent;     //top result is 100%
    int  size;
    struct tm time_tm;

} htsearch_query_match_struct;


// htsearch functions

DLLEXPORT int htsearch_open(htsearch_parameters_struct *);
DLLEXPORT int htsearch_query(htsearch_query_struct *);

DLLEXPORT int htsearch_get_nth_match(int, htsearch_query_match_struct *);
DLLEXPORT int htsearch_close();

//htsearch_free(indicator)

//DLLEXPORT char * htsearch_get_error();


#endif /* LIBHTDIG_API_H */

