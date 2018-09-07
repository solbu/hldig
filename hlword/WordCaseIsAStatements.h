//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// switch between unknown numerical types
// example usage:
// --------------------------
//  switch(word_key_info.sort[position].type) 
//  {
//#define STATEMENT(type)  case WORD_ISA_##type:pool_##type[word_key_info.sort[position].index]=val;break
//#include"WordCaseIsAStatements.h"
//  }
// --------------------------
#ifdef WORD_HAVE_TypeA
STATEMENT (TypeA);
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT (TypeB);
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT (TypeC);
#endif /* WORD_HAVE_TypeC */
#undef STATEMENT
