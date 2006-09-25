// -------------------------------------------------
// libhtdigphp.h
//
// Header File for libhtdig PHP 4.0 wrapper.
// 
// Requires libhtdig 3.2.0 or better
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// <http://www.gnu.org/copyleft/gpl.html>
//
// Copyright (c) 1995-2002 The ht://Dig Group
// Copyright (c) 2002 Rightnow Technologies, Inc.
// 
// Dual Licensed under GPL & LGPL
// See GPL and LGPL files for License.
// 
// Copies available at 
// http://www.fsf.org/licenses/gpl.html
// http://www.fsf.org/licenses/lgpl.html
//
// --------------------------------------------------

/* $Id: htdigphp.h,v 1.1.2.1 2006/09/25 22:20:30 aarnone Exp $ */

#ifndef HTDIGPHP_H
#define HTDIGPHP_H

//#if HAVE_HTDIG


#ifdef PHP_WIN32
#define PHP_HTDIG_API __declspec(dllexport)
#else
#define PHP_HTDIG_API
#endif


PHP_MINIT_FUNCTION(htdig);
PHP_MINFO_FUNCTION(htdig);

PHP_FUNCTION(htsearch_open);
PHP_FUNCTION(htsearch_query);
PHP_FUNCTION(htsearch_get_nth_match);
PHP_FUNCTION(htsearch_close);
PHP_FUNCTION(htsearch_get_error);
PHP_FUNCTION(htdig_index_test_url);


//#define DEFAULT_TIME_FORMAT      "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n"
#define DEFAULT_TIME_FORMAT      "%Y-%m-%d"

#define TIME_FORMAT_SIZE         256

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
