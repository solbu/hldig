#ifndef LIBDEFS_H
#define LIBDEFS_H

/*
   {{{ includes 
 */


#ifdef _MSC_VER /* _WIN32 */
#include <windows.h>
#endif


#include "htconfig.h"


#ifdef STDC_HEADERS
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif

/*
   }}} 
 */
/*
   {{{ typedefs 
 */

#if SIZEOF_UNSIGNED_LONG_INT == 8
typedef unsigned long word64;
#define TIGER_64BIT
#elif SIZEOF_UNSIGNED_LONG_LONG_INT == 8

#ifndef _MSC_VER /* _WIN32 */
typedef unsigned long long word64;
#else //ifdef _MSC_VER /* _WIN32 */
typedef DWORD64 word64;
#endif
#else
#error "Cannot find a 64 bit integer in your system, sorry."
#endif

#if SIZEOF_UNSIGNED_LONG_INT == 4
typedef unsigned long word32;
#elif SIZEOF_UNSIGNED_INT == 4
typedef unsigned int word32;
#else
#error "Cannot find a 32 bit integer in your system, sorry."
#endif

#if SIZEOF_UNSIGNED_INT == 2
typedef unsigned int word16;
#elif SIZEOF_UNSIGNED_SHORT_INT == 2
typedef unsigned short word16;
#else
#error "Cannot find a 16 bit integer in your system, sorry."
#endif

#if SIZEOF_UNSIGNED_CHAR == 1
typedef unsigned char word8;
#else
#error "Cannot find an 8 bit char in your system, sorry."
#endif

typedef word8 byte;
typedef word32 dword;

/*
   }}} 
 */

/*
   {{{ macros and defines 
 */

#define RAND32 (word32) ((word32)rand() << 17 ^ (word32)rand() << 9 ^ rand())

#ifndef HAVE_MEMMOVE
#ifdef HAVE_BCOPY
#define memmove(d, s, n) bcopy ((s), (d), (n))
#else
#error "Neither memmove nor bcopy exists on your system."
#endif
#endif

#define ENCRYPT 0
#define DECRYPT 1

/*
   }}} 
 */

/*
   {{{ prototypes 
 */

void Bzero(void *s, int n);

word32 byteswap(word32 x);

int BreakToThree(void *key, unsigned int keylen,
				 void *keyword1, void *keyword2, void *keyword3);

/*
   }}} 
 */

#endif
