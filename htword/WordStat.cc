//
// WordStat.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordStat.cc,v 1.2.2.1 2000/05/05 21:55:19 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordStat.h"

WordReference* WordStat::word_stat_last = 0;
