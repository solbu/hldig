//
// IntObject.cc
//
// IntObject: int variable encapsulated in Object derived class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: IntObject.cc,v 1.1.2.1 2006/09/25 23:50:31 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "IntObject.h"


//*******************************************************************************
// IntObject::IntObject()
//
IntObject::IntObject()
{
}


//*******************************************************************************
// IntObject::~IntObject()
//
IntObject::~IntObject()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "IntObject destructor start\n");

    debug->outlog(10, "IntObject destructor done\n");
}


