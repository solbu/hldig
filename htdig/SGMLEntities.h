//
// SGMLEntities.h
//
// $Id: SGMLEntities.h,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: SGMLEntities.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _SGMLEntities_h_
#define _SGMLEntities_h_

#include <Object.h>
#include <Dictionary.h>

class SGMLEntities : public Object
{
public:
    //
    // Construction/Destruction
    //
    SGMLEntities();
    ~SGMLEntities();

    static unsigned char	translate(char *);
    static unsigned char	translateAndUpdate(unsigned char *&);
	
private:
    void			init();
    Dictionary			*trans;
};

#endif


