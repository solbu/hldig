//
// HtPack: Compress for example structures.
//
// Much like the pack()/unpack() function pair in perl, but
// compressing, not "packing into a binary structure".
//
// Note that the contents of the returned "String" is not
// necessarily aligned to allow using it as a struct.
//
// $Id: HtPack.h,v 1.1 1999/03/21 15:23:56 hp Exp $
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
