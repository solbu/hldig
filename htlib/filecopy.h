//
// filecopy.h
//
//  Copies files from one file to another.
//  Contains both Unix & Native Win32 Implementations
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
// 
// Copyright (c) 2002 RightNow Technologies, Inc.
// Donated to The ht://Dig Group under LGPL License

#ifdef __cplusplus
//extern "C" {
#endif

#ifndef FILECOPY_H

#ifndef FALSE
#define FALSE       0
#endif

#ifndef TRUE
#define TRUE        1
#endif

#define FILECOPY_OVERWRITE_ON 1 
#define FILECOPY_OVERWRITE_OFF 2

int file_copy (char * from, char * to, char flags);


#ifdef __cplusplus
//}
#endif

#endif /* FILECOPY_H  */
