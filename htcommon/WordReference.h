//
// WordReference.h
//
// $Id: WordReference.h,v 1.2.2.1 1999/09/01 20:16:35 grdetil Exp $
//
// $Log: WordReference.h,v $
// Revision 1.2.2.1  1999/09/01 20:16:35  grdetil
// Change the maximum word length into a run-time option, rather than compile-time.
//
// Revision 1.2  1997/03/24 04:33:15  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include <htString.h>

class WordReference : public Object
{
public:
	//
	// Construction/Destruction
	//
					WordReference()			{}
					~WordReference()		{}

	String				Word;
	int				WordCount;
	int				Weight;
	int				Location;
	int				DocumentID;
	int				Anchor;
private:
};


#endif


