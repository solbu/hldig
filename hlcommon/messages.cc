//
// messages.cc
//
// messages: contains messages common to programs in the hldig package
//
// Part of the hl://Dig package   <https://solbu.github.io/hldig/>
// Copyright (c) 2017 The hl://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//  hl://Dig is a fork of ht://Dig <https://sourceforge.net/projects/htdig/>
//

#include <stdio.h>

#include "hlconfig.h"
#include <iostream>
#include "messages.h"

void Usage::verbose (void) {
    printf (_("\
 -v\tVerbose mode\n\
\tThis increases the verbosity of the\n\
\tprogram.  Using more than 2 is probably only useful\n\
\tfor debugging purposes.  The default verbose mode\n\
\tgives a nice progress report of what it is doing and\n\
\twhere it is.\n\n"));
}

void Usage::config (void) {
  printf (_("\
 -c\tconfigfile\n\
\tUse the specified configuration file instead of the\n\
\tdefault.\n\n"));
}

void Usage::alternate_common (void) {
  printf (_("\
 -a\tUse alternate work files.\n"));
}

Usage::Usage (void) {}

Usage::~Usage (void) {}
