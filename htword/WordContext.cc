//
// WordContext.cc 
//
// WordContext: call Initialize for all classes that need to.
//              This will enable the Instance() static member
//              of each to return a properly allocated and configured 
//              object.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordContext.cc,v 1.1.2.3 2000/01/13 14:47:09 loic Exp $
//
#include"WordContext.h"
#include"WordType.h"
#include"WordKeyInfo.h"
#include"WordRecord.h"

void 
WordContext::Initialize(const Configuration &config)
{
    WordType::Initialize(config);
    WordKeyInfo::Initialize(config);
    WordRecordInfo::Initialize(config);
}
