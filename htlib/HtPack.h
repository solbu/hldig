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
// $Id: HtPack.h,v 1.2 1999/09/08 14:42:29 loic Exp $
//

#ifndef __HtPack_h
#define __HtPack_h

#include "htString.h"

// Pack.
// The parameter "format" is not const but should normally be.
extern String
htPack(char format[], const char *theStruct);

// Unpack.
// The parameter "theString" will be updated to point after the
// processed amount of data.
extern String
htUnpack(char format[], char * &thePackedData);

#endif // __HtPack_h
