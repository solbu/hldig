//
// HtVectorGeneric.cc
//
// HtVectorGeneric: A Vector class which holds objects of type GType.
//           (A vector is an array that can expand as necessary)
//           This class is very similar in interface to the List class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtVectorGeneric.cc,v 1.1.2.1 2006/09/25 22:51:14 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtVector_int.h"
#define GType int
#define HtVectorGType HtVector_int
#include "HtVectorGenericCode.h"


#define GType char
#define HtVectorGType HtVector_char
#include "HtVectorGeneric.h"
#define GType char
#define HtVectorGType HtVector_char
#include "HtVectorGenericCode.h"

#include"HtVector_String.h"
#define GType String
#define HtVectorGType HtVector_String
#include "HtVectorGenericCode.h"


#define GType double
#define HtVectorGType HtVector_double
#include "HtVectorGeneric.h"
#define GType double
#define HtVectorGType HtVector_double
#include "HtVectorGenericCode.h"

#include<stdio.h>

// this is just to  check if it compiles ok

class ZOZO
{
    int a,b,c;
public:
    void show(){printf("ZOZO SHOW:%d %d %d\n",a,b,c);}
    ZOZO()
    {
	a=1;
	b=2;
	c=3;
    }
};


#define HTVECTORGENERIC_NOTCOMPARABLE
#define GType ZOZO
#define HtVectorGType HtVector_ZOZO
#include "HtVectorGeneric.h"
#define HTVECTORGENERIC_NOTCOMPARABLE
#define GType ZOZO
#define HtVectorGType HtVector_ZOZO
#include "HtVectorGenericCode.h"

void
test_HtVectorGeneric()
{
    HtVector_int  intv;
    HtVector_char charv;
    HtVector_ZOZO vz;
    ZOZO zz;
    vz.push_back(zz);
    vz.push_back(zz);
    vz.push_back(zz);

    int i;
    for(i=0;i<vz.size();i++)
    {
	vz[i].show();
    }
}

