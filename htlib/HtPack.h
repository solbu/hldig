//
// HtPack.h
//
// HtPack: Compress and uncompress data in e.g. simple structures.
//	   The structure must have the layout defined in the ABI;
//	   the layout the compiler generates.
//
//         Much like the pack()/unpack() function pair in perl, but
//         compressing, not "packing into a binary structure".
//
//         Note that the contents of the returned "String" is not
//         necessarily aligned to allow using it as a struct.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtPack.h,v 1.4.2.1 2000/05/10 18:23:44 loic Exp $
//

#ifndef __HtPack_h
#define __HtPack_h

#include "htString.h"

// Pack.
// The parameter "format" is not const but should normally be.
extern String
htPack(const char format[], const char *theStruct);

// Unpack.
// The parameter "theString" will be updated to point after the
// processed amount of data.
extern String
htUnpack(const char format[], const char *thePackedData);

#endif // __HtPack_h
