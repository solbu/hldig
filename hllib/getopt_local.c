/* getopt_local.c */

/* Public Domain getopt clone */

/* Part of the ht://Dig package   <http://www.htdig.org/> */
/* Copyright (c) 2003 The ht://Dig Group */
/* For copyright details, see the file COPYING in your distribution */
/* or the GNU Library General Public License (LGPL) version 2 or later or later */
/* <http://www.gnu.org/copyleft/lgpl.html> */

/*  Added to HtDig code June 2003 by Neal Richter, RightNow Technologies */

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


#include <stdio.h>

#include "getopt_local.h"

#ifdef GETOPT_LOCAL

int optind = 1;
int opterr = 1;
char *optarg = (char *) 0;

static char *next_arg = (char *) 0;

#define NO_OPT          0
#define OPT_PLAIN       1
#define OPT_ARG         2


/* ----- getopt -------------------------------------- Oct 23, 1999  21:48 ---
 */
int
getopt (int argc, char *argv[], char *optstring)
{
  int ret;
  int which = NO_OPT;

  if (next_arg == (char *) 0)
  {
    if (argv[optind] == (char *) 0 || argv[optind][0] != '-')
      return -1;
    next_arg = &argv[optind][1];
  }

  if ((*next_arg == '\0') || (*next_arg == '-'))
  {
    optind++;
    return -1;
  }

  while (*optstring)
    if (*next_arg == *optstring++)
      which = (*optstring == ':') ? OPT_ARG : OPT_PLAIN;

  switch (which)
  {
  case NO_OPT:
  case OPT_PLAIN:
    ret = *next_arg++;

    if (*next_arg == '\0')
    {
      optind++;
      next_arg = (char *) 0;
    }

    if (which == OPT_PLAIN)
      return ret;

    if (opterr)
      fprintf (stderr, "%s: illegal option -- %c\n", argv[0], ret);

    return '?';

  case OPT_ARG:
    ret = *next_arg++;
    optind++;

    if (*next_arg != '\0')
    {
      optarg = next_arg;
      next_arg = (char *) 0;
      return ret;
    }

    if (argv[optind] != (char *) 0)
    {
      optarg = argv[optind];
      optind++;
      next_arg = (char *) 0;
      return ret;
    }

    next_arg = (char *) 0;
    if (opterr)
      fprintf (stderr, "%s: option requires an option -- %c\n", argv[0], ret);
    return '?';
  }

  return (-1);
}
#elif defined(_MSC_VER)         /* _WIN32 */
#error _MSC_VER but !GETOPT_LOCAL
#endif /* GETOPT_LOCAL */
