// getopt_local.h
//
// Public Domain getopt clone
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//  Added to HtDig code June 2003 by Neal Richter, RightNow Technologies

/*
**      This getopt behaves pretty much like you would expect.
**      It does handle arguments like '-ab-' a little differently
**  then normal;  I think the -- 'stop option processing' should
**  be treated like just another option,  so that's what mine does.
**  Other getopts seem to ignore the second '-' in '-ab-'.
**
**  I hereby place this version of getopt in
**  the public domain.  Do with this what you will.
**  I'm sure there is a nicer and faster version out there
**  somewhere but I don't care!
**
**  Robert Osborne, May 1991.
*/


#ifndef GETOPT_LOCAL_H
#define GETOPT_LOCAL_H

#define GETOPT_LOCAL

#ifdef __cplusplus
extern "C" {
#endif

/* header for getopt_local.c */

extern int optind;
extern int opterr;
extern char *optarg;

int getopt(int, char *[], char *);

#ifdef __cplusplus
}
#endif

#endif /* GETOPT_LOCAL_H */
