//
// good_strtok.h
//
// good_strtok: The good_strtok() function is very similar to the 
//              standard strtok() library function, except that good_strtok() 
//		will only skip over 1 separator if it finds one.  This is
//              needed when parsing strings with empty fields.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: good_strtok.h,v 1.4.2.1 2000/05/10 18:23:45 loic Exp $
//

#ifndef	_good_strtok_h_
#define	_good_strtok_h_

char	*good_strtok(char *, char);

#endif

