/* Part of the ht://Dig package   <http://www.htdig.org/> */
/* Copyright (c) 1999-2004 The ht://Dig Group */
/* For copyright details, see the file COPYING in your distribution */
/* or the GNU Library General Public License (LGPL) version 2 or later */
/* <http://www.gnu.org/copyleft/lgpl.html> */


/*
   MD5.H - header file for MD5C.C
 */

/*
   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.
   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

#ifndef MHASH_MD5_H
#define MHASH_MD5_H

#include <libdefs.h>

/*
   PROTOTYPES should be set to one if and only if the compiler supports
   function argument prototyping.
   The following makes PROTOTYPES default to 0 if it has not already
   been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/*
   POINTER defines a generic pointer type 
 */
typedef unsigned char *POINTER;

/*
   PROTO_LIST is defined depending on how PROTOTYPES is defined above.
   If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
   returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif



/*
   MD5 context. 
 */
typedef struct
{
  word32 state[4];              /*
                                   state (ABCD) 
                                 */
  word32 count[2];              /*
                                   number of bits, modulo 2^64 (lsb first) 
                                 */
  word8 buffer[64];             /*
                                   input buffer 
                                 */
} MD5_CTX;

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST ((MD5_CTX *, const unsigned char *, unsigned int));
void *MD5Final PROTO_LIST ((MD5_CTX *));

#endif
