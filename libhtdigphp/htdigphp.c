// -------------------------------------------------
// libhtdigphp.c
//
// Code for libhtdig PHP 4.0 wrapper.
// 
// Requires libhtdig 3.2.0 or better
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// <http://www.gnu.org/copyleft/gpl.html>
//
// Copyright (c) 1995-2002 The ht://Dig Group
// Copyright (c) 2002 Rightnow Technologies, Inc.
// 
// Dual Licensed LGPL
// See LGPL file for License.
// 
// Copies available at 
// http://www.fsf.org/licenses/gpl.html
// http://www.fsf.org/licenses/lgpl.html
//
// --------------------------------------------------

/* $Id: htdigphp.c,v 1.1 2004/03/20 01:31:21 nealr Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "htdigphp.h"


/* PHP Includes */
#include "ext/standard/file.h"
#include "ext/standard/info.h"

/* for fileno() */
#include <stdio.h>
#include <time.h>

/* HtDig includes */
#include <libhtdig_api.h>

PHP_FUNCTION(htsearch_open);
PHP_FUNCTION(htsearch_query);
PHP_FUNCTION(htsearch_get_nth_match);
PHP_FUNCTION(htsearch_close);
PHP_FUNCTION(htsearch_get_error);
PHP_FUNCTION(htdig_index_test_url);

function_entry htdig_functions[] = 
{
    PHP_FE(htsearch_open, NULL)
    PHP_FE(htsearch_query, NULL)
    PHP_FE(htsearch_get_nth_match, NULL)
    PHP_FE(htsearch_close, NULL) 
    PHP_FE(htsearch_get_error, NULL) 
    PHP_FE(htdig_index_test_url, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry htdig_module_entry = 
{
    STANDARD_MODULE_HEADER,
    "htdig",
    htdig_functions,
    PHP_MINIT(htdig),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(htdig),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_HTDIG
ZEND_GET_MODULE(htdig)
#endif
//static int le_htdig;

static int num_search_results;

static char time_format[TIME_FORMAT_SIZE];

//static void php_htdig_close(zend_rsrc_list_entry * rsrc TSRMLS_DC);

PHP_MINIT_FUNCTION(htdig)
{
    /* Register the resource, with destructor (arg 1) and text description (arg 3), the 
       other arguments are just standard placeholders */
    //le_htdig = zend_register_list_destructors_ex(php_htdig_close, NULL, "HtDig 3.2.0", module_number);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(htdig)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "HtDig Support", "Enabled");
    php_info_print_table_row(2, "HtDig Version", "3.2.0b5");
    php_info_print_table_end();
}

/* 
 *  htsearch_open
 * 
 *  Wrapper for 
 *  int htsearch_open(htsearch_parameters_struct *);
 *
 *  TODO recieve configFile, debug, logFile as parameters.  2 & 3 are Optional!
 * 
 */
PHP_FUNCTION(htsearch_open)
{
    zval **config_array_arg, **pvalue;
    HashTable *config_array_ht;
    htsearch_parameters_struct htsearch_params;
    int array_size = 0;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &config_array_arg) == FAILURE)
        WRONG_PARAM_COUNT;

    if (Z_TYPE_PP(config_array_arg) != IS_ARRAY)
    {
        php_error(E_WARNING, "First argument to htsearch_open() should be an array");
        return;
    }

    convert_to_array_ex(config_array_arg);

    config_array_ht = Z_ARRVAL_PP(config_array_arg);

    array_size = zend_hash_num_elements(config_array_ht);

    if (array_size < 3)
    {
        php_error(E_WARNING, "First argument to htsearch_open() should be an array with at least 3 elements");
        return;
    }

    memset(&htsearch_params ,0, sizeof(htsearch_params));
    
    //----------- Required Paramters --------------

    if (zend_hash_find(config_array_ht, "configFile", sizeof("configFile"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.configFile, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'configFile' key not in array parameter");
        return;
    }

    if (zend_hash_find(config_array_ht, "debug", sizeof("debug"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_long_ex(pvalue);
        htsearch_params.debug = Z_LVAL_PP(pvalue);
    }
    else
    {
        php_error(E_WARNING, "'debug' key not in array parameter");
        return;
    }

    if (zend_hash_find(config_array_ht, "logFile", sizeof("logFile"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.logFile, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'logFile' key not in array parameter");
        return;
    }


    //----------- Optional Paramters --------------
    
    if (zend_hash_find(config_array_ht, "DBpath", sizeof("DBpath"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.DBpath, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.DBpath[0] = 0;  //NULL terminator
        htsearch_params.DBpath[1] = 0;
    }

    if (zend_hash_find(config_array_ht, "locale", sizeof("locale"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.locale, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.locale[0] = 0;  //NULL terminator
        htsearch_params.locale[1] = 0;
    }
    
    if (zend_hash_find(config_array_ht, "restrict", sizeof("restrict"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.search_restrict, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.search_restrict[0] = 0;  //NULL terminator
        htsearch_params.search_restrict[1] = 0;
    }
    
    if (zend_hash_find(config_array_ht, "exclude", sizeof("exclude"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.search_exclude, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.search_exclude[0] = 0;  //NULL terminator
        htsearch_params.search_exclude[1] = 0;
    }
    
    if (zend_hash_find(config_array_ht, "alwaysret", sizeof("alwaysret"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.search_alwaysreturn, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.search_alwaysreturn[0] = 0;  //NULL terminator
        htsearch_params.search_alwaysreturn[1] = 0;
    }

    if (zend_hash_find(config_array_ht, "title_factor", sizeof("title_factor"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.title_factor, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.title_factor[0] = 0;  //NULL terminator
        htsearch_params.title_factor[1] = 0;
    }

    if (zend_hash_find(config_array_ht, "text_factor", sizeof("text_factor"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.text_factor, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.text_factor[0] = 0;  //NULL terminator
        htsearch_params.text_factor[1] = 0;
    }

    if (zend_hash_find(config_array_ht, "meta_description_factor", sizeof("meta_description_factor"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htsearch_params.meta_description_factor, Z_STRVAL_PP(pvalue));
    }
    else
    {
        htsearch_params.meta_description_factor[0] = 0;  //NULL terminator
        htsearch_params.meta_description_factor[1] = 0;
    }


    
    snprintf(time_format, TIME_FORMAT_SIZE, "%s", DEFAULT_TIME_FORMAT);

    htsearch_open(&htsearch_params);

    num_search_results = 0;

    RETURN_LONG(SUCCESS);

}

/* 
 *  wrapper for htsearch_query(htsearch_query_struct *)
 * 
 *  TODO recieve query, algorithms_flag, sortby_flag, format as parameters
 *  Some Optional??
 *
 *       "raw_query" => "$p_query"
 *       "algorithm" => HTSEARCH_ALG_AND_STR
 *       "sortby" => HTSEARCH_SORT_SCORE_STR
 *       "format" => HTSEARCH_FORMAT_LONG_STR
 *       "time_format"                          **** optional *** 
 *
 *
 */
PHP_FUNCTION(htsearch_query)
{
    zval **query_array_arg, **pvalue;
    HashTable *query_array_ht;
    htsearch_query_struct the_query;
    int array_size = 0;

    num_search_results = 0;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &query_array_arg) == FAILURE)
        WRONG_PARAM_COUNT;

    if (Z_TYPE_PP(query_array_arg) != IS_ARRAY)
    {
        php_error(E_WARNING, "First argument to htsearch_query() should be an array");
        return;
    }


    //SEPARATE_ZVAL(query_array_arg);
    convert_to_array_ex(query_array_arg);

    query_array_ht = Z_ARRVAL_PP(query_array_arg);

    array_size = zend_hash_num_elements(query_array_ht);

    if (array_size < 4)
    {
        php_error(E_WARNING, "First argument to htsearch_open() should be an array with at least 4 elements");
        return;
    }

    if (zend_hash_find(query_array_ht, "raw_query", sizeof("raw_query"), 
                        (void **) &pvalue) == SUCCESS)
    {
        //SEPARATE_ZVAL(pvalue);
        convert_to_string_ex(pvalue);
        sprintf(the_query.raw_query, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'raw_query' key not in array parameter");
        return;
    }

    if (zend_hash_find(query_array_ht, "algorithm", sizeof("algorithm"), 
                        (void **) &pvalue) == SUCCESS)
    {
        //SEPARATE_ZVAL(pvalue);
        convert_to_string_ex(pvalue);

        if (strcmp(HTSEARCH_ALG_AND_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.algorithms_flag = HTSEARCH_ALG_AND;
        else if (strcmp(HTSEARCH_ALG_OR_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.algorithms_flag = HTSEARCH_ALG_OR;
        else if (strcmp(HTSEARCH_ALG_BOOLEAN_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.algorithms_flag = HTSEARCH_ALG_BOOLEAN;
        else
        {
            php_error(E_WARNING, "'algorithm' value not valid [%s]", Z_STRVAL_PP(pvalue));
            return;
        }
    }
    else
    {
        php_error(E_WARNING, "'algorithm' key not in array parameter");
        return;
    }

    if (zend_hash_find(query_array_ht, "format", sizeof("format"), 
                        (void **) &pvalue) == SUCCESS)
    {
        //SEPARATE_ZVAL(pvalue);
        convert_to_string_ex(pvalue);

        if (strcmp(HTSEARCH_FORMAT_LONG_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.format = HTSEARCH_FORMAT_LONG;
        else if (strcmp(HTSEARCH_FORMAT_SHORT_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.format = HTSEARCH_FORMAT_SHORT;
        else
        {
            php_error(E_WARNING, "'format' value not valid [%s]", Z_STRVAL_PP(pvalue));
            return;
        }

    }
    else
    {
        php_error(E_WARNING, "'format' key not in array parameter");
        return;
    }

    if (zend_hash_find(query_array_ht, "sortby", sizeof("sortby"), 
                        (void **) &pvalue) == SUCCESS)
    {
        //SEPARATE_ZVAL(pvalue);
        convert_to_string_ex(pvalue);

        if (strcmp(HTSEARCH_SORT_SCORE_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_SCORE;
        else if (strcmp(HTSEARCH_SORT_REV_SCORE_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_REV_SCORE;
        else if (strcmp(HTSEARCH_SORT_TIME_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_TIME;
        else if (strcmp(HTSEARCH_SORT_REV_TIME_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_REV_TIME;
        else if (strcmp(HTSEARCH_SORT_TITLE_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_TITLE;
        else if (strcmp(HTSEARCH_SORT_REV_TITLE_STR, Z_STRVAL_PP(pvalue)) == 0)
            the_query.sortby_flag = HTSEARCH_SORT_REV_TITLE;
        else
        {
            php_error(E_WARNING, "'sortby' value not valid [%s]", Z_STRVAL_PP(pvalue));
            return;
        }

    }
    else
    {
        php_error(E_WARNING, "'sortby' key not in array parameter");
        return;
    }

    if (zend_hash_find(query_array_ht, "time_format", sizeof("time_format"), 
                        (void **) &pvalue) == SUCCESS)
    {
        //SEPARATE_ZVAL(pvalue);
        convert_to_string_ex(pvalue);

        snprintf(time_format, TIME_FORMAT_SIZE, "%s", Z_STRVAL_PP(pvalue));
    }

    
    num_search_results = htsearch_query(&the_query);

    //printf("[%s][%d]\n", the_query.raw_query, num_results);

    RETURN_LONG(num_search_results);
    
}

/* 
 *
 * 
 */
PHP_FUNCTION(htsearch_get_nth_match)
{
    zval **result_num_arg, **pvalue;
    htsearch_query_match_struct result;
    int result_num;
    int ret = 0;
    char local_time_str[TIME_FORMAT_SIZE];

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &result_num_arg) == FAILURE)
        WRONG_PARAM_COUNT;

    if (Z_TYPE_PP(result_num_arg) != IS_LONG)
    {
        php_error(E_WARNING, "First argument to htsearch_get_nth_match() should be an integer");
        return;
    }

    convert_to_long_ex(result_num_arg);
    result_num = Z_LVAL_PP(result_num_arg);

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    
    ret = htsearch_get_nth_match(result_num, &result);

    if(ret > 0)
    {

        strftime(local_time_str, TIME_FORMAT_SIZE, time_format, &result.time_tm);
        //fprintf(stderr, "time:[%s]\n", asctime(&result.time_tm));

        add_assoc_string(return_value, "title",          result.title,         1);
        add_assoc_string(return_value, "URL",            result.URL,           1);
        add_assoc_string(return_value, "excerpt",        result.excerpt,       1);
        add_assoc_string(return_value, "time",           local_time_str,       1);
        add_assoc_long(  return_value, "score",          result.score           );
        add_assoc_long(  return_value, "scorepercent",   result.score_percent   );
        add_assoc_long(  return_value, "size",           result.size            );
        add_assoc_long(  return_value, "size",           result.size            );

        //fprintf(stderr, "%s\n", result.title);
    }
    else
    {
        add_assoc_long(  return_value, "error",           ret            );
    }
    
}

/* 
 * 
 *
 */
PHP_FUNCTION(htsearch_close)
{
    int ret = 0;

    ret = htsearch_close();

    RETURN_LONG(ret);
}

/* 
 * 
 *
 */
PHP_FUNCTION(htsearch_get_error)
{
    RETURN_LONG(SUCCESS);
}


PHP_FUNCTION(htdig_index_test_url)
{
    zval **config_array_arg, **pvalue;
    zval *tmp;
    HashTable *config_array_ht;
    htdig_parameters_struct htdigparms;
    int array_size = 0;
    int ret = 0;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &config_array_arg) == FAILURE)
        WRONG_PARAM_COUNT;

    if (Z_TYPE_PP(config_array_arg) != IS_ARRAY)
    {
        php_error(E_WARNING, "First argument to htdig_index_test_url() should be an array");
    }

    convert_to_array_ex(config_array_arg);

    config_array_ht = Z_ARRVAL_PP(config_array_arg);

    array_size = zend_hash_num_elements(config_array_ht);

    if (array_size < 1)
    {
        php_error(E_WARNING, "First argument to htdig_index_test_url() should be an array with at least 1 elements");
        return;
    }

    MAKE_STD_ZVAL(tmp);
    
    memset(&htdigparms ,0, sizeof(htdigparms));
    
    //configFile
    if (zend_hash_find(config_array_ht, "configFile", sizeof("configFile"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.configFile, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'configFile' key not in array parameter");
        return;
    }

    //URL
    if (zend_hash_find(config_array_ht, "URL", sizeof("URL"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.URL, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'URL' key not in array parameter");
        return;
    }

    //limit_urls_to
    if (zend_hash_find(config_array_ht, "limit_urls_to", sizeof("limit_urls_to"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.limit_urls_to, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'limit_urls_to' key not in array parameter");
        return;
    }

    //limit_normalized
    if (zend_hash_find(config_array_ht, "limit_normalized", sizeof("limit_normalized"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.limit_normalized, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'limit_normalized' key not in array parameter");
        return;
    }


    //exclude_urls
    if (zend_hash_find(config_array_ht, "exclude_urls", sizeof("exclude_urls"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.exclude_urls, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'exclude_urls' key not in array parameter");
        return;
    }
    

    //search_restrict
    if (zend_hash_find(config_array_ht, "search_restrict", sizeof("search_restrict"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.search_restrict, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'search_restrict' key not in array parameter");
        return;
    }
    

    //search_exclude
    if (zend_hash_find(config_array_ht, "search_exclude", sizeof("search_exclude"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.search_exclude, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'search_exclude' key not in array parameter");
        return;
    }

    
    //url_rewrite_rules
    if (zend_hash_find(config_array_ht, "url_rewrite_rules", sizeof("url_rewrite_rules"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.url_rewrite_rules, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'url_rewrite_rules' key not in array parameter");
        return;
    }

    //bad_querystr
    if (zend_hash_find(config_array_ht, "bad_querystr", sizeof("bad_querystr"), 
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.bad_querystr, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'bad_querystr' key not in array parameter");
        return;
    }

    //max_hop_count
    if (zend_hash_find(config_array_ht, "max_hop_count", sizeof("max_hop_count"),
                        (void **) &pvalue) == SUCCESS)
    {
        convert_to_string_ex(pvalue);
        sprintf(htdigparms.max_hop_count, Z_STRVAL_PP(pvalue));
    }
    else
    {
        php_error(E_WARNING, "'max_hop_count' key not in array parameter");
        return;
    }

    
    //now test the URL with these filters
    ret = htdig_index_test_url(&htdigparms);
    
    //add_assoc_string(*config_array_arg, "rewritten_URL", htdigparms.rewritten_URL, 1);
    
    if (zend_hash_find(config_array_ht, "rewritten_URL", sizeof("rewritten_URL"),
                        (void **) &pvalue) == SUCCESS)
    {
        //update rewritten_URL in hashtable-array
        ZVAL_STRING(tmp, htdigparms.rewritten_URL, TRUE);
        zend_hash_update(config_array_ht, "rewritten_URL", sizeof("rewritten_URL"), 
                          (void *) &tmp, sizeof(zval *), NULL);
    }
    else
    {
        //add rewritten_URL to hashtable-array
        ZVAL_STRING(tmp, htdigparms.rewritten_URL, TRUE);
        zend_hash_add(config_array_ht, "rewritten_URL", sizeof("rewritten_URL"), 
                          (void *) &tmp, sizeof(zval *), NULL);
    }
    
    //test add/update
    if (zend_hash_find(config_array_ht, "rewritten_URL", sizeof("rewritten_URL"),
                        (void **) &pvalue) != SUCCESS)
    {
        RETURN_LONG(-9999999);
    }
    
    RETURN_LONG(ret);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 tw=78 fdm=marker
 * vim<600: sw=4 ts=4 tw=78
 */
