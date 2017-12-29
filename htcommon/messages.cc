//
// messages.cc
//
// messages: contains messages common to programs in the hldig package
//
// Part of the hl://Dig package   <https://andy5995.github.io/hldig/>
// Copyright (c) 2017 The hl://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//  hl://Dig is a fork of ht://Dig <https://sourceforge.net/projects/htdig/>
//

#include <stdio.h>

#include "htconfig.h"
#include <iostream>
#include "messages.h"

void Usage::verbose (void) {
    printf (_("\t-v\tVerbose mode.  This increases the verbosity of the\n\
\t\tprogram.  Using more than 2 is probably only useful\n\
\t\tfor debugging purposes.  The default verbose mode\n\
\t\tgives a nice progress report of what it is doing and\n\
\t\twhere it is.\n\n"));
}

void Usage::config (void) {

  printf (_("\t-c configfile\n\
\t\tUse the specified configuration file instead of the\n\
\t\tdefault.\n\n"));
}

Usage::Usage (void) {}

Usage::~Usage (void) {}
